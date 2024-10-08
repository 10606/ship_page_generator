#ifndef CONNECTION_H
#define CONNECTION_H

#include <array>
#include <cctype>
#include <charconv>
#include <stddef.h>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <numeric>
#include <vector>
#include <iostream>
#include <deque>
#include <stdint.h>

#include <unistd.h>

#include "buffer.h"
#include "connection_helper.h"
#include "file_to_send.h"
#include "simple_string.h"


namespace limits
{
static const constexpr size_t first_line = 1024;
static const constexpr size_t headers_cnt = 100;
static const constexpr size_t header_size = 1024;
};


template <typename handler_t, typename socket_t>
struct connection
{
    template <typename ... Args>
    connection (handler_t && _handler, Args && ... args) :
        handler(std::move(_handler)),
        direction(get_request),
        read_state(first_line),
        client_to_server(),
        client_read(client_to_server.safe_begin()),
        client_parsed(client_to_server.safe_begin()),

        method{client_to_server.safe_begin(), client_to_server.safe_begin()},
        uri(client_to_server.safe_begin(), client_to_server.safe_begin()),
        headers(),
        content_length(std::numeric_limits <size_t> ::max()),
        keep_alive(0),
        
        server_to_client_queue(),
        server_to_client_pos(0),
        zero_copy_counter(0),
        socket(std::forward <Args> (args) ...)
    {}

    connection (connection && other) :
        handler(std::move(other.handler)),
        direction(std::move(other.direction)),
        read_state(std::move(other.read_state)),
        client_to_server(std::move(other.client_to_server)),
        client_read(std::move(other.client_read)),
        client_parsed(std::move(other.client_parsed)),
        
        method(std::move(other.method)),
        uri(std::move(other.uri)),
        headers(std::move(other.headers)),
        content_length(other.content_length),
        keep_alive(other.keep_alive),

        server_to_client_queue(std::move(other.server_to_client_queue)),
        server_to_client_pos(other.server_to_client_pos),
        zero_copy_counter(other.zero_copy_counter),
        socket(std::move(other.socket))
    {
        client_read.buf = &client_to_server;
        client_parsed.buf = &client_to_server;
        method.set_buffer(client_to_server);
        uri.set_buffer(client_to_server);
        
        for (header_t & header : headers)
        {
            header.key.set_buffer(client_to_server);
            header.value.set_buffer(client_to_server);
        }
    }

    connection (connection const & other) = delete;
    connection & operator = (connection && other) = delete;
    connection & operator = (connection const & other) = delete;

    bool read ()
    {
        if (!socket.connected())
        {
            socket.do_accept();
            return socket.want_read();
        }
        
        if (read_state != read_body)
        {
            size_t rb = socket.read(client_to_server.get_buffer());
            client_to_server.written(rb);
            if (rb == 0)
                return 1;
        }
        else
        {
            char buffer[4096];
            size_t rb = socket.read({buffer, std::min(content_length, std::extent_v <decltype(buffer)>)});
            handler.read(*this, std::string_view(buffer, rb));
            content_length -= rb;
            if (content_length == 0)
                end_read(1);
            return content_length != 0;
        }
        return process();
    }
        
    bool process ()
    {
        buffer_t::overflow_iterator readed = client_read.overflow();
        buffer_t::overflow_iterator parsed = client_parsed.overflow();
        switch (read_state)
        {
        case first_line:
        {
            buffer_t::overflow_iterator end_first_line = find(readed, client_to_server.overflow_end(), '\n');
            if (end_first_line == client_to_server.overflow_end())
            {
                if (client_to_server.size() > limits::first_line)
                    throw std::runtime_error("too long first line");
                client_read = buffer_t::safe_iterator(client_to_server, end_first_line);
                break;
            }
            else if (end_first_line == parsed || *(end_first_line - 1) != '\r')
                throw std::runtime_error("wrong first line: LF without CR");
            else
            {
                buffer_t::overflow_iterator end_method = find(parsed, end_first_line, ' ');
                if (end_method == end_first_line)
                    throw std::runtime_error("wrong first line: method");
                method = {client_parsed, buffer_t::safe_iterator(client_to_server, end_method)};
                for (; end_method != end_first_line && std::isspace(*end_method); end_method++);
                buffer_t::overflow_iterator end_path = find(end_method, end_first_line, ' ');
                if (end_path == end_method || end_path == end_first_line)
                    throw std::runtime_error("wrong first line: method and path");
                uri = {buffer_t::safe_iterator(client_to_server, end_method), buffer_t::safe_iterator(client_to_server, end_path)};
                for (; end_path != end_first_line && std::isspace(*end_path); end_path++);
                if (find(end_path, end_first_line, ' ') != end_first_line)
                    throw std::runtime_error("wrong first line: proto");
                readed = ++end_first_line;
                parsed = readed;
                client_parsed = buffer_t::safe_iterator(client_to_server, readed);
                client_read = client_parsed;
                read_state = read_headers;
                [[fallthrough]];
            }
        }
        case read_headers:
        {
            bool repeat = 1;
            bool need_break = 1;
            while (repeat)
            {
                buffer_t::overflow_iterator end_cur_header = find(readed, client_to_server.overflow_end(), '\n');
                if (end_cur_header == client_to_server.overflow_end())
                {
                    if (client_to_server.safe_end() - client_parsed > limits::header_size)
                        throw std::runtime_error("too long header");
                    client_read = buffer_t::safe_iterator(client_to_server, end_cur_header);
                    repeat = 0;
                }
                else if (end_cur_header == parsed || *(end_cur_header - 1) != '\r')
                    throw std::runtime_error("wrong header: LF without CR");
                else if (end_cur_header == parsed + 1)
                {
                    readed = ++end_cur_header;
                    parsed = readed;
                    client_parsed = buffer_t::safe_iterator(client_to_server, parsed);
                    client_read = client_parsed;

                    {
                        std::string overflow_case; // we can get overflow only once
                        std::string_view method_sv = method.to_string_view(overflow_case);
                        std::string_view uri_sv = uri.to_string_view(overflow_case);

                        if (method_sv != "POST" &&
                            method_sv != "PUT" &&
                            method_sv != "PATCH" &&
                            method_sv != "DELETE" &&
                            method_sv != "CONNECT")
                            content_length = 0;
                        keep_alive = 0;
                        
                        std::vector <std::pair <std::string_view, std::string_view> > headers_sv;
                        headers_sv.reserve(headers.size());
                        for (header_t & header : headers)
                        {
                            headers_sv.emplace_back(header.to_string_view(overflow_case));
                            if (headers_sv.back().first == "Content-Length")
                            {
                                size_t parsed_content_length;
                                std::from_chars_result res = std::from_chars(headers_sv.back().second.data(),
                                                                             headers_sv.back().second.data() + headers_sv.back().second.size(),
                                                                             parsed_content_length);
                                if (res.ec == std::errc())
                                    content_length = parsed_content_length;
                            }
                            if (headers_sv.back().first == "Connection" &&
                                headers_sv.back().second == "keep-alive")
                                keep_alive = 1;
                            // std::cerr << headers_sv.back().first << " " << headers_sv.back().second << std::endl;
                        }
                        handler.handle_head(*this, method_sv, uri_sv, headers_sv);   
                    }
                    
                    read_state = read_body;
                    repeat = 0;
                    need_break = 0;
                }
                else
                {
                    if (headers.size() > limits::headers_cnt)
                        throw std::runtime_error("too many headers");
                    buffer_t::overflow_iterator end_key = find(parsed, end_cur_header, ':');
                    if (end_key == end_cur_header)
                        throw std::runtime_error("wrong header: key");
                    buffer_t::overflow_iterator begin_value = end_key;
                    begin_value++;
                    for (; begin_value != end_cur_header && std::isspace(*begin_value); begin_value++);
                    safe_view key = {client_parsed, buffer_t::safe_iterator(client_to_server, end_key)};
                    safe_view value =
                    {
                        buffer_t::safe_iterator(client_to_server, begin_value),
                        buffer_t::safe_iterator(client_to_server, end_cur_header - 1)
                    };
                    headers.emplace_back(key, value);
                    readed = ++end_cur_header;
                    parsed = readed;
                    client_parsed = buffer_t::safe_iterator(client_to_server, parsed);
                    client_read = client_parsed;
                }
            }
            if (need_break)
                break;
            [[fallthrough]];
        }
        case read_body:
            // start read body
            size_t length = std::min(content_length, client_to_server.overflow_end() - readed);
            if (readed != client_to_server.overflow_end())
            {
                std::pair <buffer_t::iterator, buffer_t::iterator> first = first_part(readed, readed + length);
                std::pair <buffer_t::iterator, buffer_t::iterator> second = second_part(readed, readed + length);
                if (first.first != first.second)
                    handler.read(*this, std::string_view(first.first, first.second));
                if (second.first != second.second)
                    handler.read(*this, std::string_view(second.first, second.second));
            }
            client_to_server.readed(readed - client_to_server.overflow_begin());
            client_to_server.readed(length);
            content_length -= length;
            client_read = client_to_server.safe_begin();
            client_parsed = client_to_server.safe_begin();
            method = {client_to_server.safe_begin(), client_to_server.safe_begin()};
            uri = {client_to_server.safe_begin(), client_to_server.safe_begin()};
            headers.clear();
            if (content_length == 0)
                end_read(1);
            return content_length == 0;
        }
        
        return 1;
    }
    
    bool write ()
    {
        if (!socket.connected())
        {
            socket.do_accept();
            return socket.want_write();
        }
        if (!have_to_send())
            return 0;
        if (!server_to_client().data.empty())
        {
            auto [wb, is_zero_copy] = socket.write(server_to_client().data.get_data());
            server_to_client().data.readed(wb);
            if (is_zero_copy)
                server_to_client().zero_copy_set_range(zero_copy_counter);
        }
        else
            socket.send_file(server_to_client().file_to_send);
        bool answer = have_to_send();
        do_on_keep_alive();
        return answer;
    }

    void set_keep_alive (bool flag) noexcept
    {
        keep_alive = flag;
    }
    
    bool get_keep_alive () const noexcept
    {
        return keep_alive;
    }
    
    void do_on_keep_alive ()
    {
        if (!want_read() && !want_write() && keep_alive)
            direction = get_request;
    }
    
    void end_read (bool keep_alive_flag = 0)
    {
        handler.end_read(*this);
        direction = send_response;
        read_state = first_line;
        keep_alive &= keep_alive_flag;
        do_on_keep_alive();
    }
    
    template <typename ... Args>
    void send (Args && ... args)
    {
        server_to_client_queue.push_back(data_to_send{.data = buffer_t(std::forward <Args> (args) ...)});
        // direction = send_response;
    }
    
    void send_file (std::string_view file_name)
    {
        simple_string response;
        file_to_send_t file_to_send;
        try
        {
            // Range
            content_range_t content_range(headers);
            file_to_send = file_to_send_t(file_name, content_range.start, content_range.size);
            char time_buffer[1 + std::numeric_limits <decltype(file_to_send.mtime)> ::digits10 + 2];
            std::to_chars_result length_time = std::to_chars(time_buffer + 1, time_buffer + sizeof(time_buffer) - 1, file_to_send.mtime);
            time_buffer[0] = '\"';
            length_time.ptr[0] = '\"';
            std::string_view mtime_cmp(time_buffer, length_time.ptr + 1);
            bool cached = 0;
            for (header_t & header : headers)
            {
                std::string overflow_case; // we can get overflow only once
                std::pair <std::string_view, std::string_view> header_sv = header.to_string_view(overflow_case);
                if (header_sv.first == "If-None-Match" &&
                    header_sv.second == mtime_cmp)
                    cached = 1;
            }
            
            auto set_keep_alive = [this, &response] () ->void
            {
                if (get_keep_alive())
                    response.append("\r\nConnection: keep-alive");
                else
                    response.append("\r\nConnection: close");
            };
            
            if (!cached)
            {
                if (file_to_send.size != file_to_send.total_size || file_to_send.offset != 0)
                {
                    response.append
                    (
                        "HTTP/1.1 206 Partial Content\r\n",
                        "Content-Range: bytes ",
                        std::to_string(file_to_send.offset),
                        "-",
                        std::to_string(file_to_send.size - 1),
                        "/",
                        std::to_string(file_to_send.total_size)
                    );
                }
                else
                {
                    response.append("HTTP/1.1 200 OK");
                }
                set_keep_alive();
                response.append
                (
                    "\r\nContent-Length: ",
                    std::to_string(file_to_send.size - file_to_send.offset),
                    "\r\nEtag: ",
                    mtime_cmp,
                    "\r\n\r\n"
                );
            }
            else
            {
                response.append("HTTP/1.1 304 Not Modified");
                set_keep_alive();
                response.append("\r\nContent-Length: 0\r\n\r\n");
                file_to_send = file_to_send_t();
            }
        }
        catch (...)
        {
            response.append("HTTP/1.1 403 Forbidden\r\n\r\n");
            file_to_send = file_to_send_t();
        }
        size_t header_size = response.size();
        server_to_client_queue.push_back(data_to_send{.data = buffer_t(response.reset(), header_size), .file_to_send = std::move(file_to_send)});
        // direction = send_response;
    }
    
    bool want_write () const noexcept
    {
        if (!socket.connected())
            return socket.want_write();
        return have_to_send();
    }
    
    bool in_server_to_client_limit () const noexcept
    {
        return server_to_client_queue.size() < server_to_client_limit;
    }
    
    bool want_read () const noexcept
    {
        if (!socket.connected())
            return socket.want_read();
        return in_server_to_client_limit() && (keep_alive || direction == get_request);
    }
    
    bool can_read () const noexcept
    {
        return in_server_to_client_limit() && socket.can_read();
    }
    
    bool can_process () noexcept
    {
        return in_server_to_client_limit() && client_read != client_to_server.safe_end();
    }
    
    bool want_wait () const noexcept
    {
        have_to_send();
        return server_to_client_pos != 0;
    }

    handler_t handler;
    
    enum
    {
        get_request,
        send_response
    } direction;
    
    enum 
    {
        first_line,
        read_headers,
        read_body,
    } read_state;

    buffer_t client_to_server;
    buffer_t::safe_iterator client_read;
    buffer_t::safe_iterator client_parsed;
    
    safe_view method;
    safe_view uri;
    
    std::vector <header_t> headers;
    size_t content_length;
    bool keep_alive;
    
    struct data_to_send
    {
        buffer_t data;
        file_to_send_t file_to_send;
        
        zero_copy_range_t zero_copy_range = {0, 0}; // first, count
        uint32_t already_free = 0;
        
        bool can_remove () const noexcept
        {
            return zero_copy_range.count == already_free;
        }

        void zero_copy_set_range (uint32_t & value)
        {
            if (!zero_copy_range.count)
                zero_copy_range.from = value;
            zero_copy_range.count++;
            value++;
        }
        
        void free (zero_copy_range_t range)
        {
            if (range.from < zero_copy_range.from)
            {
                if (range.from + range.count <= zero_copy_range.from)
                    return;
                size_t intersect_size = range.count - (zero_copy_range.from - range.from);
                already_free += std::min(intersect_size, zero_copy_range.count);
            }
            else
            {
                if (zero_copy_range.from + zero_copy_range.count <= range.from)
                    return;
                size_t intersect_size = zero_copy_range.count - (range.from - zero_copy_range.from);
                already_free += std::min(intersect_size, range.count);
            }
        }
    };
    mutable std::deque <data_to_send> server_to_client_queue;
    mutable size_t server_to_client_pos;
    uint32_t zero_copy_counter;
    static const constexpr size_t server_to_client_limit = 4;
    
    data_to_send & server_to_client () const
    {
        return server_to_client_queue[server_to_client_pos];
    }
    
    bool have_to_send () const noexcept
    {
        while (!server_to_client_queue.empty())
        {
            if (server_to_client_queue.front().data.empty() &&
                !server_to_client_queue.front().file_to_send.want_write() &&
                server_to_client_queue.front().can_remove())
            {
                server_to_client_queue.pop_front();
                if (server_to_client_pos)
                    server_to_client_pos--;
            }
            else
                break;
        }
        while (server_to_client_pos != server_to_client_queue.size())
        {
            if (server_to_client().data.empty() &&
                !server_to_client().file_to_send.want_write())
                server_to_client_pos++;
            else
                break;
        }
        return server_to_client_pos != server_to_client_queue.size();
    }
    
    bool notify_err ()
    {
        std::optional <zero_copy_range_t> zero_copy_range = socket.notify_err();
        if (!zero_copy_range)
            return 0;
        for (data_to_send & data : server_to_client_queue)
            data.free(*zero_copy_range);
        return 1;
    }
    
    socket_t socket;
    
    static const constexpr 
    std::array <std::string_view, 9> methods =
    {{
        "GET",
        "HEAD",
        "POST",
        "PUT",
        "PATCH",
        "DELETE",
        "CONNECT",
        "OPTIONS",
        "TRACE"
    }};
    
    static const constexpr size_t max_method_len = 
        std::accumulate
        (
            methods.begin(), 
            methods.end(), 
            0, 
            [] (size_t ans, std::string_view cur) -> size_t 
            {
                return std::max(ans, cur.size());
            }
        );
};


#endif

