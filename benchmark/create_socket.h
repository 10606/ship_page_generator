#ifndef CREATE_SOCKET_H
#define CREATE_SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <stdexcept>
#include <unistd.h>
#include <optional>


struct get_addr 
{
    get_addr () = delete;
    
    get_addr (char const * addr, char const * port)
    {
        addrinfo hints;

        memset(&hints, 0, sizeof(addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;

        int ret = getaddrinfo(addr, port, &hints, &result);
        if (ret != 0)
            throw std::runtime_error("cant' parse address");
        
        current = result;
    }
    
    get_addr (get_addr const &) = delete;
    get_addr (get_addr &&) = delete;
    get_addr & operator = (get_addr const &) = delete;
    get_addr & operator = (get_addr &&) = delete;
    
    addrinfo const * get () noexcept
    {
        addrinfo * ans = current;
        
        if (current)
            current = current->ai_next;
        
        return ans;
    }
    
    void reset ()
    {
        current = result;
    }
    
    ~get_addr ()
    {
        freeaddrinfo(result);
    }
    
private:
    addrinfo * result;
    addrinfo * current;
};


struct connect_socket
{
    connect_socket (char * addr, char * port, bool _is_ssl) :
        connect_socket(get_addr(addr, port), _is_ssl)
    {}
    
    template <typename get_addr_ref>
    requires std::is_same_v <std::remove_reference_t <get_addr_ref>, get_addr>
    connect_socket (get_addr_ref && addresses, bool _is_ssl) :
        fd(-1),
        is_ssl(_is_ssl)
    {
        while (addrinfo const * addr = addresses.get())
        {
            try
            {
                connect_socket socket = connect_socket(addr, _is_ssl);
                if (!socket.valid())
                    continue;
                swap(socket);
                return;
            }
            catch (...)
            {
                continue;
            }
            break;
        }
        
        throw std::runtime_error("can't create socket");
    }

    connect_socket (addrinfo const * addr, bool _is_ssl) :
        is_ssl(_is_ssl)
    {
        fd = socket(addr->ai_family, SOCK_STREAM | SOCK_NONBLOCK, addr->ai_protocol);
        if (fd == -1)
            // throw std::runtime_error("can't create tcp socket");
            return;
        while (1)
        {
            int ret = connect(fd, addr->ai_addr, addr->ai_addrlen);
            if (ret == 0 || errno == EINPROGRESS)
                break;
            if (errno == EAGAIN || errno == EINTR)
                continue;
            close();
            // throw std::runtime_error("can't connect on tcp socket");
            return;
        }
    }
    
    connect_socket (connect_socket const &) = delete;
    
    connect_socket (connect_socket && other) noexcept : 
        fd(other.fd),
        is_ssl(other.is_ssl)
    {
        other.fd = -1;
    }
    
    connect_socket & operator = (connect_socket const &) = delete;
    
    connect_socket & operator = (connect_socket && rhs) noexcept
    {
        close();
        fd = rhs.fd;
        rhs.fd = -1;
        is_ssl = rhs.is_ssl;
        return *this;
    }
    
    ~connect_socket () noexcept
    {
        close();
    }
    
    void swap (connect_socket & other)
    {
        std::swap(fd, other.fd);
    }
    
    enum status_t
    {
        ok = 0,
        error,
        again,
    };
    
    status_t check () noexcept
    {
        int status;
        socklen_t status_len = sizeof(status);
        int ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &status, &status_len);
        if (ret != 0)
            return error;
        if (status == 0 || status == EISCONN)
            return ok;
        if (status == EINPROGRESS || status == EAGAIN || status == EINTR)
            return again;
        return error;
    }
    
    int move_fd () noexcept
    {
        int answer = fd;
        fd = -1;
        return answer;
    }

    bool valid () const noexcept
    {
        return fd != -1;
    }
    
    void close ()
    {
        if (fd != -1)
            ::close(fd);
        fd = -1;
    }
    
    int fd;
    bool is_ssl;
};




#endif

