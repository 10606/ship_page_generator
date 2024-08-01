#ifndef BENCHMARK_CONNECTION_H
#define BENCHMARK_CONNECTION_H

#include "driver/buffer.h"
#include "driver/connection_helper.h"

#include <charconv>
#include <map>
#include <openssl/ssl.h>
#include <sys/epoll.h>
#include <tuple>
#include <deque>


namespace limits
{
static const constexpr size_t first_line = 1024;
static const constexpr size_t header_size = 1024;
};


template <typename socket_t>
struct connection
{
    connection (int fd) :
        buffer(),
        response_read(buffer.safe_begin()),
        response_parsed(buffer.safe_begin()),
        
        length(),
        keep_alive(0),
        read_id(0),
        read_pos(0),
        state(first_line),
        
        send_data(),
        send_id(0),
        send_pos(0),
        
        socket(fd)
    {}
 
    connection (connection &&) = delete;
    connection (connection const &) = delete;
    
    connection & operator = (connection &&) = delete;
    connection & operator = (connection const &) = delete;
    
    bool connected () const noexcept
    {
        return socket.connected();
    }
    
    bool want_read () const noexcept
    {
        if (!connected())
            return socket.want_read();
        return send_id > read_id && (state != body || !length || read_pos < *length);
    }
    
    bool want_write () const noexcept
    {
        if (!connected())
            return socket.want_write();
        return !send_data.empty() && send_pos < send_data.front().size();
    }
 
    bool can_read () noexcept
    {
        return socket.can_read();
    }
    
    bool can_process () noexcept
    {
        return response_read != buffer.safe_end();
    }
    
    void read ()
    {
        if (!want_read())
            return;
        if (!connected())
        {
            socket.do_connect();
            return;
        }
        if (state != body)
        {
            size_t rb = socket.read(buffer.get_buffer());
            buffer.written(rb);
        }
        process();
        if (state == body)
        {
            char tmp_buffer[256 * 1024];
            size_t need_read = sizeof(tmp_buffer);
            if (length)
                need_read = std::min(need_read, *length - read_pos);
            if (need_read)
            {
                size_t rb = socket.read(std::span(tmp_buffer, need_read));
                read_pos += rb;
                read_bytes_body += rb;
                read_bytes_total += rb;
            }
            if (length && read_pos == *length)
            {
                // response readed
                read_id++;
                state = first_line;
                length.reset();
                keep_alive = 0;
                read_pos = 0;
            }
        }
    }
        
    void process ()
    {
        buffer_t::overflow_iterator readed = response_read.overflow();
        buffer_t::overflow_iterator parsed = response_parsed.overflow();
        switch (state)
        {
        case first_line:
        {
            buffer_t::overflow_iterator end_first_line = find(readed, buffer.overflow_end(), '\n');
            if (end_first_line == buffer.overflow_end())
            {
                if (buffer.size() > limits::first_line)
                    throw std::runtime_error("too long first line");
                response_read = buffer_t::safe_iterator(buffer, end_first_line);
                break;
            }
            else if (end_first_line == parsed || *(end_first_line - 1) != '\r')
                throw std::runtime_error("wrong first line: LF without CR");
            else
            {
                buffer_t::overflow_iterator end_proto = find(parsed, end_first_line, ' ');
                if (end_proto == end_first_line)
                    throw std::runtime_error("wrong first line: method");
                // proto = {response_parsed, buffer_t::safe_iterator(buffer, end_proto)};
                for (; end_proto != end_first_line && std::isspace(*end_proto); end_proto++);
                buffer_t::overflow_iterator end_code = find(end_proto, end_first_line, ' ');
                if (end_code == end_proto || end_code == end_first_line)
                    throw std::runtime_error("wrong first line: method and path");
                // code = {buffer_t::safe_iterator(buffer, end_proto), buffer_t::safe_iterator(buffer, end_code)};
                for (; end_code != end_first_line && std::isspace(*end_code); end_code++);
                readed = ++end_first_line;
                parsed = readed;
                response_parsed = buffer_t::safe_iterator(buffer, readed);
                response_read = response_parsed;
                state = headers;
                [[fallthrough]];
            }
        }
        case headers:
        {
            bool repeat = 1;
            bool need_break = 1;
            while (repeat)
            {
                buffer_t::overflow_iterator end_cur_header = find(readed, buffer.overflow_end(), '\n');
                if (end_cur_header == buffer.overflow_end())
                {
                    if (buffer.safe_end() - response_parsed > limits::header_size)
                        throw std::runtime_error("too long header");
                    response_read = buffer_t::safe_iterator(buffer, end_cur_header);
                    repeat = 0;
                }
                else if (end_cur_header == parsed || *(end_cur_header - 1) != '\r')
                    throw std::runtime_error("wrong header: LF without CR");
                else if (end_cur_header == parsed + 1)
                {
                    readed = ++end_cur_header;
                    parsed = readed;
                    response_parsed = buffer_t::safe_iterator(buffer, parsed);
                    response_read = response_parsed;
                    
                    state = body;
                    repeat = 0;
                    need_break = 0;
                }
                else
                {
                    buffer_t::overflow_iterator end_key = find(parsed, end_cur_header, ':');
                    if (end_key == end_cur_header)
                        throw std::runtime_error("wrong header: key");
                    buffer_t::overflow_iterator begin_value = end_key;
                    begin_value++;
                    for (; begin_value != end_cur_header && std::isspace(*begin_value); begin_value++);
                    
                    safe_view key = {response_parsed, buffer_t::safe_iterator(buffer, end_key)};
                    safe_view value =
                    {
                        buffer_t::safe_iterator(buffer, begin_value),
                        buffer_t::safe_iterator(buffer, end_cur_header - 1)
                    };
                    
                    std::string overflow_case; // we can get overflow only once
                    std::string_view key_str = key.to_string_view(overflow_case);
                    std::string_view value_str = value.to_string_view(overflow_case);

                    if (key_str == "Content-Length")
                    {
                        size_t parsed_content_length;
                        std::from_chars_result res = std::from_chars(value_str.data(), value_str.data() + value_str.size(), parsed_content_length);
                        if (res.ec == std::errc())
                            length = parsed_content_length;
                    }
                    if (length && key_str == "Connection" && value_str == "keep-alive")
                        keep_alive = 1;
                    // std::cerr << key_str << " " << value_str << std::endl;

                    readed = ++end_cur_header;
                    parsed = readed;
                    response_parsed = buffer_t::safe_iterator(buffer, parsed);
                    response_read = response_parsed;
                }
            }
            if (need_break)
                break;
            [[fallthrough]];
        }
        case body:
            size_t already_have = buffer.overflow_end() - readed;
            if (length)
                already_have = std::min(already_have, *length);
            read_bytes_total += readed - buffer.overflow_begin();
            buffer.readed(readed - buffer.overflow_begin());
            buffer.readed(already_have);
            read_pos += already_have;
            read_bytes_body += already_have;
            read_bytes_total += already_have;
            response_read = buffer.safe_begin();
            response_parsed = buffer.safe_begin();

            if (length && read_pos == *length)
            {
                // response readed
                read_id++;
                state = first_line;
                length.reset();
                keep_alive = 0;
                read_pos = 0;
            }
            break;
        }
    }
    
    void write ()
    {
        if (!want_write())
            return;
        if (!connected())
        {
            socket.do_connect();
            return;
        }
        size_t wb = socket.write(std::span(send_data.front().substr(send_pos)));
        send_pos += wb;
        if (send_pos == send_data.front().size())
        {
            send_id++;
            send_data.pop_front();
            send_pos = 0;
        }
    }
    
    void send (std::string_view data)
    {
        send_data.push_back(data);
    }
    
    
    uint64_t get_sent () const noexcept
    {
        return send_id;
    }
    
    size_t get_queued () const noexcept
    {
        return send_data.size();
    }
    
    int fd () const noexcept
    {
        return socket.fd;
    }
    
    uint64_t complete_requests () const noexcept
    {
        return read_id;
    }
    
    uint64_t read_bytes_body = 0;
    uint64_t read_bytes_total = 0;
    
private:
    enum read_state
    {
        first_line,
        headers,
        body,
    };
    
    buffer_t buffer;
    buffer_t::safe_iterator response_read;
    buffer_t::safe_iterator response_parsed;
    
    std::optional <size_t> length;
    bool keep_alive;
    uint64_t read_id;
    size_t read_pos; // body
    read_state state;
    
    std::deque <std::string_view> send_data;
    uint64_t send_id;
    size_t send_pos;
    
    socket_t socket;
};


#endif

