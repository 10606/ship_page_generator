#ifndef RAW_SOCKET_H
#define RAW_SOCKET_H

#include <stddef.h>
#include <span>
#include <stdexcept>
#include <iostream>
#include <cstring>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <linux/errqueue.h>
#include <errno.h>
#include <linux/if_packet.h>


struct raw_socket
{
    raw_socket (int _fd) :
        fd(_fd)
    {
        if (fd == -1)
            throw std::runtime_error("socket fd = -1");
        fcntl(fd, F_SETFL, O_NONBLOCK);
        
        int one = 1;
        setsockopt(fd, SOL_SOCKET, SO_ZEROCOPY, &one, sizeof(one));
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    }
    
    raw_socket (raw_socket && other) noexcept :
        fd(other.fd)
    {
        other.fd = -1;
    }

    raw_socket & operator = (raw_socket && other) noexcept 
    {
        close();
        fd = other.fd;
        other.fd = -1;
        return *this;
    }
    
    ~raw_socket () noexcept
    {
        close();
    }
    
    raw_socket (raw_socket const &) = delete;
    raw_socket & operator = (raw_socket const &) = delete;

    size_t read (std::span <char> buffer)
    {
        ssize_t ret = ::read(fd, buffer.data(), buffer.size());
        if (ret == -1)
        {
            if (errno == EINTR ||
                errno == EAGAIN)
                return 0;
            throw std::runtime_error("raw error read");
        }
        return ret;
    }
    
    size_t write (std::span <const char> buffer)
    {
        ssize_t ret = ::write(fd, buffer.data(), buffer.size());
        if (ret == -1)
        {
            if (errno == EINTR ||
                errno == EAGAIN)
                return 0;
            throw std::runtime_error("raw error read");
        }
        return ret;
    }
    
    bool connected () const noexcept
    {
        return 1;
    }
    
    bool want_read () const noexcept
    {
        return 0;
    }
    
    bool want_write () const noexcept
    {
        return 0;
    }
    
    bool can_read () const noexcept
    {
        return 0;
    }
    
    void do_connect ()
    {}
    
    void close () noexcept
    {
        if (fd != -1)
            ::close(fd);
    }
    
    int fd;
};

#endif


