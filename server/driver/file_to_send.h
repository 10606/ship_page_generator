#ifndef FILE_TO_SEND_H
#define FILE_TO_SEND_H

#include <stddef.h>
#include <stdexcept>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>


struct file_to_send_t
{
    file_to_send_t () noexcept :
        fd(-1),
        offset(0),
        size(0)
    {}
    
    file_to_send_t (int _fd) :
        fd(_fd),
        offset(0)
    {
        if (fd == -1)
            throw std::runtime_error("file to send fd = -1");
        struct stat stat_value;
        int ret = fstat(fd, &stat_value);
        if (ret == -1)
            throw std::runtime_error("can't get file size");
        size = stat_value.st_size;
    }

    file_to_send_t (std::string_view path) :
        file_to_send_t(::open(path.data(), 0))
    {}
    
    file_to_send_t (file_to_send_t && other) noexcept :
        fd(other.fd),
        offset(other.offset),
        size(other.size)
    {
        other.fd = -1;
        other.offset = 0;
        other.size = 0;
    }
    
    file_to_send_t & operator = (file_to_send_t && other) noexcept
    {
        if (fd != -1)
            ::close(fd);
        fd = other.fd;
        offset = other.offset;
        size = other.size;
        other.fd = -1;
        other.offset = 0;
        other.size = 0;
        return *this;
    }

    file_to_send_t (file_to_send_t const & other) = delete;
    file_to_send_t & operator = (file_to_send_t const & other) = delete;
    
    ~file_to_send_t () noexcept
    {
        if (fd != -1)
            ::close(fd);
    }
    
    bool want_write () const noexcept
    {
        return offset != size;
    }
    
    int fd;
    off_t offset;
    off_t size;
};


#endif

