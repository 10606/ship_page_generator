#ifndef SSL_SOCKET_H
#define SSL_SOCKET_H

#include <stddef.h>
#include <span>
#include <stdexcept>
#include <iostream>
#include <atomic>
#include <mutex>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


struct ssl_wrap
{
    ssl_wrap () :
        ssl_ctx(nullptr),
        ssl(nullptr)
    {
        if (!init)
        {
            SSL_library_init();
            init = 1;
        }
        ssl_ctx = SSL_CTX_new(TLS_client_method());
        
        if (!ssl_ctx)
            throw std::runtime_error("can't initialize ssl ctx");
        SSL_CTX_set_options(ssl_ctx, SSL_OP_ENABLE_KTLS);
        ssl = SSL_new(ssl_ctx);
        if (!ssl)
        {
            close();
            throw std::runtime_error("can't initialize ssl");
        }
        SSL_set_options(ssl, SSL_OP_NO_SSLv2);
        SSL_set_options(ssl, SSL_OP_NO_SSLv3);
        SSL_set_options(ssl, SSL_OP_NO_TLSv1);
        SSL_set_options(ssl, SSL_OP_NO_TLSv1_1);
    }
    
    ~ssl_wrap ()
    {
        close();
    }

    static ssl_wrap & get ()
    {
        static ssl_wrap answer;
        return answer;
    }
    
    void close () noexcept
    {
        if (ssl)
            SSL_free(ssl);
        if (ssl_ctx)
            SSL_CTX_free(ssl_ctx);
    }
    
    static bool init;
    
    SSL_CTX * ssl_ctx;
    SSL * ssl;
};

struct ssl_socket
{
    ssl_socket (int _fd) :
        fd(_fd),
        ssl(SSL_dup(ssl_wrap::get().ssl)),
        need_read(0),
        need_write(0)
    {
        if (fd == -1)
        {
            close();
            throw std::runtime_error("socket fd = -1");
        }
        if (!ssl)
        {
            close();
            throw std::runtime_error("can't create SSL");
        }
        if (!SSL_set_fd(ssl, fd))
        {
            close();
            throw std::runtime_error("can't set SSL fd");
        }
        SSL_set_connect_state(ssl);
        need_read = 1;
        need_write = 1;
        /*
        if (SSL_connect(ssl) <= 0)
        {
            close();
            throw std::runtime_error("SSL accept error");
        }
        */
        
        fcntl(fd, F_SETFL, O_NONBLOCK);
        
        int one = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    }
    
    ssl_socket (ssl_socket && other) noexcept :
        fd(other.fd),
        ssl(other.ssl),
        need_read(other.need_read),
        need_write(other.need_write)
    {
        other.fd = -1;
        other.ssl = nullptr;
        other.need_read = 0;
        other.need_write = 0;
    }

    ssl_socket & operator = (ssl_socket && other) noexcept
    {
        close();
        fd = other.fd;
        ssl = other.ssl;
        need_read = other.need_read;
        need_write = other.need_write;
        
        other.fd = -1;
        other.ssl = nullptr;
        other.need_read = 0;
        other.need_write = 0;
        return *this;
    }
    
    ~ssl_socket () noexcept
    {
        close();
    }
    
    ssl_socket (ssl_socket const &) = delete;
    ssl_socket & operator = (ssl_socket const &) = delete;

    size_t read (std::span <char> buffer)
    {
        check_error();
        int ret = SSL_read(ssl, buffer.data(), buffer.size());
        if (ret <= 0)
        {
            if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_READ)
                return 0;
            if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_WRITE)
                return 0;
            throw std::runtime_error("ssl error read");
        }
        return ret;
    }
    
    size_t write (std::span <const char> buffer)
    {
        check_error();
        int ret = SSL_write(ssl, buffer.data(), buffer.size());
        if (ret <= 0)
        {
            if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_WRITE)
                return 0;
            throw std::runtime_error("ssl error write");
        }
        return ret;
    }
    
    bool connected () const noexcept
    {
        return !need_read && !need_write;
    }
    
    bool want_read () const noexcept
    {
        return need_read;
    }
    
    bool want_write () const noexcept
    {
        return need_write;
    }
    
    bool can_read () const noexcept
    {
        return SSL_pending(ssl);
    }
    
    void do_connect ()
    {
        check_error();
        if (!want_read() && !want_write())
            return;
        need_write = 0;
        need_read = 0;
        int ret = SSL_do_handshake(ssl);
        if (ret <= 0)
        {
            if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_WRITE)
            {
                need_write = 1;
                return;
            }
            if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_READ)
            {
                need_read = 1;
                return;
            }
            throw std::runtime_error("ssl handshake error");
        }
    }
    
    void close () noexcept
    {
        if (ssl)
        {
            ERR_clear_error();
            if (!(SSL_get_shutdown(ssl) & SSL_RECEIVED_SHUTDOWN))
                SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        if (fd != -1)
            ::close(fd);
    }
    
    void check_error ()
    {
        ERR_clear_error();
        if (SSL_get_shutdown(ssl) & SSL_RECEIVED_SHUTDOWN)
            throw std::runtime_error("ssl shutdowned");
    }
    
    int fd;
    SSL * ssl;
    bool need_read;
    bool need_write;
};


#endif

