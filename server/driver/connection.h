#ifndef CONNECTION_H
#define CONNECTION_H

#include <stddef.h>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <numeric>
#include <vector>
#include <iostream>

#include <unistd.h>

#include "buffer.h"
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
        
        server_to_client(),
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

        server_to_client(std::move(other.server_to_client)),
        socket(std::move(other.socket))
    {
        client_read.buf = &client_to_server;
        client_parsed.buf = &client_to_server;
        method.first.buf = &client_to_server;
        method.second.buf = &client_to_server;
        uri.first.buf = &client_to_server;
        uri.second.buf = &client_to_server;
        
        for (header_t & header : headers)
        {
            header.key.first.buf = &client_to_server;
            header.key.second.buf = &client_to_server;
            header.value.first.buf = &client_to_server;
            header.value.second.buf = &client_to_server;
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
        
        if (direction == send_response)
            return 0;

        if (read_state != read_body)
        {
            size_t rb = socket.read(client_to_server.get_buffer());
            client_to_server.written(rb);
            if (rb == 0)
                return 1;
        }
        else
        {
            // TODO content length
            char buffer[4096];
            size_t rb = socket.read({buffer, std::min(content_length, std::extent_v <decltype(buffer)>)});
            handler.read(*this, std::string_view(buffer, rb));
            if (content_length == 0)
                end_read();
            return content_length != 0;
        }
        
        buffer_t::iterator readed = client_read.unsafe();
        buffer_t::iterator parsed = client_parsed.unsafe();
        switch (read_state)
        {
        case first_line:
        {
            buffer_t::iterator end_first_line = std::find(readed, client_to_server.end(), '\n');
            if (end_first_line == client_to_server.end())
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
                buffer_t::iterator end_method = std::find(parsed, end_first_line, ' ');
                if (end_method == end_first_line)
                    throw std::runtime_error("wrong first line: method");
                method = {client_parsed, buffer_t::safe_iterator(client_to_server, end_method)};
                for (; end_method != end_first_line && std::isspace(*end_method); end_method++);
                buffer_t::iterator end_path = std::find(end_method, end_first_line, ' ');
                if (end_path == end_method || end_path == end_first_line)
                    throw std::runtime_error("wrong first line: method and path");
                uri = {buffer_t::safe_iterator(client_to_server, end_method), buffer_t::safe_iterator(client_to_server, end_path)};
                for (; end_path != end_first_line && std::isspace(*end_path); end_path++);
                if (std::find(end_path, end_first_line, ' ') != end_first_line)
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
                buffer_t::iterator end_cur_header = std::find(readed, client_to_server.end(), '\n');
                if (end_cur_header == client_to_server.end())
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
                    client_parsed = buffer_t::safe_iterator(client_to_server, readed);
                    client_read = client_parsed;

                    {
                        std::string_view method_sv(method.first.unsafe(), method.second.unsafe());
                        std::string_view uri_sv(uri.first.unsafe(), uri.second.unsafe());
                        
                        std::vector <std::pair <std::string_view, std::string_view> > headers_sv;
                        headers_sv.reserve(headers.size());
                        for (header_t & header : headers)
                            headers_sv.emplace_back(header.to_string_view());
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
                    buffer_t::iterator end_key = std::find(parsed, end_cur_header, ':');
                    if (end_key == end_cur_header)
                        throw std::runtime_error("wrong header: key");
                    buffer_t::iterator begin_value = end_key;
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
                    client_parsed = buffer_t::safe_iterator(client_to_server, readed);
                    client_read = client_parsed;
                }
            }
            if (need_break)
                break;
            [[fallthrough]];
        }
        case read_body:
            size_t length = std::min(content_length, static_cast <size_t> (client_to_server.end() - readed));
            if (readed != client_to_server.end())
                handler.read(*this, std::string_view(readed, length));
            
            client_to_server.readed(length);
            client_read = client_to_server.safe_begin();
            client_parsed = client_to_server.safe_begin();
            method = {client_to_server.safe_begin(), client_to_server.safe_begin()};
            uri = {client_to_server.safe_begin(), client_to_server.safe_begin()};
            headers.clear();
            break;
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
        if (!server_to_client.empty())
        {
            size_t wb = socket.write(server_to_client.get_data());
            server_to_client.readed(wb);
            return (!server_to_client.empty() || file_to_send.want_write());
        }
        else
        {
            socket.send_file(file_to_send);
            return file_to_send.want_write();
        }
    }
    
    void end_read ()
    {
        handler.end_read(*this);
        direction = send_response;
    }
    
    template <typename ... Args>
    void send (Args && ... args)
    {
        server_to_client = buffer_t(std::forward <Args> (args) ...);
        direction = send_response;
    }
    
    void send_file (std::string_view file_name)
    {
        simple_string response;
        try
        {
            // TODO Range
            file_to_send = file_to_send_t(file_name);
            std::string mtime_str = std::to_string(file_to_send.mtime);
            std::string mtime_cmp = "\"" + mtime_str + "\"";
            bool cached = 0;
            for (header_t & header : headers)
            {
                std::pair <std::string_view, std::string_view> header_sv = header.to_string_view();
                if (header_sv.first == "If-None-Match" &&
                    header_sv.second == mtime_cmp)
                    cached = 1;
            }
            
            if (!cached)
            {
                response.append("HTTP/1.1 200 OK\r\n")
                        .append("Content-Length: ")
                        .append(std::to_string(file_to_send.size))
                        .append("\r\nEtag: \"")
                        .append(mtime_str)
                        .append("\"\r\n\r\n");
            }
            else
            {
                response.append("HTTP/1.1 304 Not Modified\r\nContent-Length: 0\r\n\r\n");
                file_to_send = file_to_send_t();
            }
        }
        catch (...)
        {
            response.append("HTTP/1.1 403 Forbidden\r\n\r\n");
            file_to_send = file_to_send_t();
        }
        size_t header_size = response.size();
        server_to_client = buffer_t(response.reset(), header_size);
        direction = send_response;
    }
    
    bool want_write () const noexcept
    {
        if (!socket.connected())
            return socket.want_write();
        return direction == send_response && 
               (!server_to_client.empty() || file_to_send.want_write());
    }
    
    bool want_read () const noexcept
    {
        if (!socket.connected())
            return socket.want_read();
        return direction == get_request;
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
    
    using safe_view = std::pair <buffer_t::safe_iterator, buffer_t::safe_iterator>;
    safe_view method;
    safe_view uri;
    
    struct header_t
    {
        header_t
        (
            safe_view _key,
            safe_view _value
        ) :
            key(_key),
            value(_value)
        {}
    
        std::pair <std::string_view, std::string_view> to_string_view ()
        {
            return
            {
                std::string_view(  key.first.unsafe(),   key.second.unsafe()),
                std::string_view(value.first.unsafe(), value.second.unsafe())
            };
        }

        safe_view key;
        safe_view value;
    };
    std::vector <header_t> headers;
    size_t content_length;
    
    buffer_t server_to_client;
    file_to_send_t file_to_send;
    
    socket_t socket;
    
    static const constexpr 
    std::array <std::string_view, 2> methods = 
    {{
        "GET",
        "POST"
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
