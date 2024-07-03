#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <span>
#include <cstring>
#include <set>
#include <algorithm>
#include <numeric>
#include <cctype>

#include <sys/socket.h>
#include <netinet/in.h>

#include "epoll_wrap.h"
#include "raw_socket.h"
#include "ssl_socket.h"
#include "connection.h"


template <typename handler_t>
struct server
{
    struct port_descr
    {
        in_port_t port;
        bool is_ssl;
    };

    server
    (
        std::vector <port_descr> ports,
        std::string_view cert = {}, 
        std::string_view key = {}
    ) :
        ssl(cert, key)
    {
        for (port_descr port : ports)
            open_socket(port);
    }
    
    ~server ()
    {
        for (auto fd : server_fds)
        {
            epoll.del(fd.first);
            ::close(fd.first);
        }
        
        for (auto const & conn : raw_connections)
            epoll.del(conn.first);
        for (auto const & conn : ssl_connections)
            epoll.del(conn.first);
        raw_connections.clear();
        ssl_connections.clear();
    }

    using socket_handler_t = decltype(std::declval <handler_t> ().accept());
    template <typename socket_t>
    using connection_t = connection <socket_handler_t, socket_t>;
    template <typename socket_t>
    using connections_storage = std::map <int, connection_t <socket_t> >;
    
    void open_socket (port_descr port)
    {
        int fd = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (fd == -1)
            throw std::runtime_error("can't create server socket");
        in6_addr addr_raw;
        memset(addr_raw.s6_addr, 0, 16);
        sockaddr_in6 addr{AF_INET6, htons(port.port), 0, addr_raw, 0};

        int reuse = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));

        int ret = bind(fd, reinterpret_cast <struct sockaddr *> (&addr), sizeof(addr));
        if (ret == -1)
        {
            close(fd);
            throw std::runtime_error("can't bind server socket");
        }
        ret = listen(fd, 100000);
        if (ret == -1)
        {
            close(fd);
            throw std::runtime_error("can't listen on socket");
        }
        
        try
        {
            epoll.add(fd, EPOLLIN);
        }
        catch (...)
        {
            close(fd);
            throw;
        }

        try
        {
            server_fds.insert({fd, port.is_ssl});
        }
        catch (...)
        {
            epoll.del(fd);
            close(fd);
            throw;
        }
    }
    
    void accept (epoll_wrap::envoke event, bool is_ssl)
    {
        if (event.events & EPOLLERR)
        {
            epoll.del(event.fd);
            server_fds.erase(event.fd);
            close(event.fd);
        }
        int fd = ::accept(event.fd, NULL, NULL);
        if (fd != -1)
        {
            try
            {
                auto tmp = handler.accept();
                epoll.add(fd, EPOLLIN);
                int fd_moved = fd;
                fd = -1;
                try
                {
                    if (!is_ssl)
                        raw_connections.insert({fd_moved, connection_t <raw_socket> (std::move(tmp), fd_moved)});
                    else
                        ssl_connections.insert({fd_moved, connection_t <ssl_socket> (std::move(tmp), fd_moved, ssl)});
                }
                catch (std::exception & e)
                {
                    // std::cerr << e.what() << std::endl;
                    epoll.del(fd_moved);
                }
            }
            catch (std::exception & e)
            {
                // std::cerr << e.what() << std::endl;
                if (fd != -1)
                    close(fd);
            }
        }
    }
    
    template <typename socket_t>
    void process
    (
        epoll_wrap::envoke event,
        connections_storage <socket_t> & connections,
        typename connections_storage <socket_t> ::iterator it_raw
    )
    {
        bool removed = 0;
        auto remove = [this, &connections, it_raw, &removed] () -> void
        {
            if (removed)
                return;
            epoll.del(it_raw->first);
            connections.erase(it_raw);
            removed = 1;
        };
        
        try
        {
            if (event.events & EPOLLIN)
                do
                {
                    it_raw->second.read();
                } while (it_raw->second.can_read());
            if (event.events & EPOLLOUT)
                it_raw->second.write();
            if (event.events & EPOLLRDHUP)
                it_raw->second.end_read();
            if (event.events & EPOLLERR)
                if (!it_raw->second.notify_err())
                    remove();
            if (event.events & EPOLLHUP)
                remove();
            if (!removed)
            {
                uint32_t mask = 0;
                if (it_raw->second.want_read())
                    mask |= EPOLLIN;
                if (it_raw->second.want_write())
                    mask |= EPOLLOUT;
                if (mask == 0 && !it_raw->second.want_wait())
                    remove();
                else
                    epoll.mod(it_raw->first, mask);
            }
        }
        catch (std::exception & e)
        {
            // std::cerr << e.what() << std::endl;
            remove();
        }
    }
    
    void process (int timeout)
    {
        epoll_wrap::envoke events[100];
        size_t count = epoll.wait(events, timeout);
        for (size_t i = 0; i != count; ++i)
        {   
            std::map <int, bool> ::iterator it_server = server_fds.find(events[i].fd);
            if (it_server != server_fds.end())
            {
                accept(events[i], it_server->second);
                continue;
            }
            
            typename connections_storage <raw_socket> ::iterator it_raw = 
                raw_connections.find(events[i].fd);
            if (it_raw != raw_connections.end())
            {
                process <raw_socket> (events[i], raw_connections, it_raw);
                continue;
            }

            typename connections_storage <ssl_socket> ::iterator it_ssl = 
                ssl_connections.find(events[i].fd);
            if (it_ssl != ssl_connections.end())
            {
                process <ssl_socket> (events[i], ssl_connections, it_ssl);
                continue;
            }
        }
    }
    
    
    handler_t handler;
    epoll_wrap epoll;
    std::map <int, bool> server_fds; // is_ssl
    ssl_wrap ssl;
    connections_storage <raw_socket> raw_connections;
    connections_storage <ssl_socket> ssl_connections;
};


#endif

