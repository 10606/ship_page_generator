#ifndef RAW_SOCKET_H
#define RAW_SOCKET_H

#include <stddef.h>
#include <span>
#include <stdexcept>
#include <iostream>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "file_to_send.h"


struct raw_socket
{
    raw_socket (int _fd) :
        fd(_fd)
    {
        if (fd == -1)
            throw std::runtime_error("socket fd = -1");
        fcntl(fd, F_SETFL, O_NONBLOCK);
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
            throw std::runtime_error("error read");
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
            throw std::runtime_error("error read");
        }
        return ret;
    }
    
    void send_file (file_to_send_t & file)
    {
        ssize_t ret = sendfile(fd, file.fd, &file.offset, file.size - file.offset);
        if (ret == -1)
        {
            if (errno == EINTR ||
                errno == EAGAIN)
                return;
            perror("send file");
            throw std::runtime_error("error send file");
        }
    }
    
    bool connected () const noexcept
    {
        return 1;
    }
    
    bool want_read () const noexcept
    {
        return 1;
    }
    
    bool want_write () const noexcept
    {
        return 1;
    }
    
    void do_accept ()
    {}
    
    void close () noexcept
    {
        if (fd != -1)
            ::close(fd);
    }
    
    int fd;
};

#endif

