#ifndef RAW_SOCKET_H
#define RAW_SOCKET_H

#include <stddef.h>
#include <span>
#include <stdexcept>
#include <iostream>
#include <cstring>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <linux/errqueue.h>
#include <errno.h>
#include <linux/if_packet.h>

#include "file_to_send.h"


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
    
    // <size written, is_zero_copy (need safe buffer)>
    std::pair <size_t, bool> write (std::span <const char> buffer)
    {
        ssize_t ret;
        // zero copy disabled
        // it got worse in localhost, zero copy is piece of shit
        // with:    3 * 10^6 KB/s
        // without: 4 * 10^6 KB/s
        ret = -1; // ::send(fd, buffer.data(), buffer.size(), MSG_ZEROCOPY);
        if (ret >= 0)
        {
            // std::cerr << "zerocopy send " << ret << std::endl;
            return {ret, 1};
        }
        ret = ::write(fd, buffer.data(), buffer.size());
        if (ret == -1)
        {
            if (errno == EINTR ||
                errno == EAGAIN)
                return {0, 0};
            throw std::runtime_error("error read");
        }
        return {ret, 0};
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
    
    bool can_read () const noexcept
    {
        return 0;
    }
    
    void do_accept ()
    {}
    
    std::optional <zero_copy_range_t> notify_err ()
    {
        // zero copy
        char control[100];
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_control = control;
        msg.msg_controllen = sizeof(control);
        int ret = recvmsg(fd, &msg, MSG_ERRQUEUE);
        if (ret == -1)
        {
            /*
            std::cerr << "err recvmsg " << errno << " " << strerror(errno) << std::endl;
            if (errno == EAGAIN)
                std::cerr << "(can retry)" << std::endl;
            */
            return std::nullopt;
        }
        
        struct cmsghdr * cm = CMSG_FIRSTHDR(&msg);
        if ((cm->cmsg_level != SOL_IP   || cm->cmsg_type != IP_RECVERR) &&
            (cm->cmsg_level != SOL_IPV6 || cm->cmsg_type != IPV6_RECVERR) &&
            (cm->cmsg_level != SOL_PACKET || cm->cmsg_type != PACKET_TX_TIMESTAMP))
        {
            // std::cerr << "cmsghdr not SOL_IP IP_RECVERR " << cm->cmsg_level << " " << cm->cmsg_type << std::endl;
            return std::nullopt;
        }
        
        struct sock_extended_err * serr = reinterpret_cast <sock_extended_err *> (CMSG_DATA(cm));
        if (serr->ee_errno != 0 ||
            serr->ee_origin != SO_EE_ORIGIN_ZEROCOPY)
        {
            // std::cerr << "sock_extended_err not SO_EE_ORIGIN_ZEROCOPY" << std::endl;
            return std::nullopt;
        }
        
        // std::cerr << "notify: " << serr->ee_info << " " << serr->ee_data << std::endl;
        return zero_copy_range_t{serr->ee_info, static_cast <size_t> (serr->ee_data - serr->ee_info) + 1};
    }
    
    void close () noexcept
    {
        if (fd != -1)
            ::close(fd);
    }
    
    int fd;
};

#endif

