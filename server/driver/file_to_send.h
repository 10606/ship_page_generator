#ifndef FILE_TO_SEND_H
#define FILE_TO_SEND_H

#include <optional>
#include <stddef.h>
#include <stdexcept>
#include <stdint.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>


struct file_to_send_t
{
    file_to_send_t () noexcept :
        fd(-1),
        offset(0),
        size(0),
        total_size(0),
        mtime(0)
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
        total_size = stat_value.st_size;
        size = total_size;
        mtime = stat_value.st_mtim.tv_sec;
    }

    file_to_send_t (int _fd, off_t start, std::optional <off_t> len = std::nullopt) :
        file_to_send_t(_fd)
    {
        if (start != 0 && start <= size)
        {
            off_t ret = lseek(fd, start, SEEK_SET);
            if (ret != -1)
                offset = ret;
            if (len)
                *len += start - offset;
        }
        if (len && *len < 0)
            *len = 0;
        if (len && size - offset > *len)
            size = offset + *len;
    }

    file_to_send_t (std::string_view path) :
        file_to_send_t(::open(path.data(), 0))
    {}

    file_to_send_t (std::string_view path, off_t start, std::optional <off_t> len = std::nullopt) :
        file_to_send_t(::open(path.data(), 0), start, len)
    {}
    
    file_to_send_t (file_to_send_t && other) noexcept :
        fd(other.fd),
        offset(other.offset),
        size(other.size),
        total_size(other.total_size),
        mtime(other.mtime)
    {
        other.fd = -1;
        other.offset = 0;
        other.size = 0;
        other.total_size = 0;
        other.mtime = 0;
    }
    
    file_to_send_t & operator = (file_to_send_t && other) noexcept
    {
        if (fd != -1)
            ::close(fd);
        fd = other.fd;
        offset = other.offset;
        size = other.size;
        total_size = other.total_size;
        mtime = other.mtime;
        other.fd = -1;
        other.offset = 0;
        other.size = 0;
        other.total_size = 0;
        other.mtime = 0;
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
    off_t total_size;
    time_t mtime;
};


struct zero_copy_range_t
{
    uint32_t from;
    size_t count;
};


#endif

