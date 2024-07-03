#ifndef SSL_SOCKET_H
#define SSL_SOCKET_H

#include <stddef.h>
#include <span>
#include <stdexcept>
#include <iostream>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "file_to_send.h"


struct ssl_wrap
{
    ssl_wrap (std::string_view cert, std::string_view key) :
        ssl_ctx(SSL_CTX_new(TLS_server_method())),
        ssl(nullptr)
    {
        if (!init)
        {
            SSL_library_init();
            init = 1;
        }
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
        
        if (!cert.empty())
        {
            if (key.empty()) 
                key = cert;
            if (SSL_use_certificate_file(ssl, cert.data(), 1) != 1)
            {
                close();
                throw std::runtime_error("can't use cert file");
            } 
            if (SSL_use_PrivateKey_file(ssl, key.data(), 1) != 1) 
            {
                close();
                throw std::runtime_error("can't use key file");
            } 
            if (SSL_use_certificate_chain_file(ssl, cert.data()) != 1) 
            {
                close();
                throw std::runtime_error("can't set cert as chain");
            }
        }
    }
    
    ~ssl_wrap ()
    {
        close();
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
    ssl_socket (int _fd, ssl_wrap & _ssl) :
        fd(_fd),
        ssl(SSL_dup(_ssl.ssl)),
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
        SSL_set_accept_state(ssl);
        need_read = 1;
        need_write = 1;
        /*
        if (SSL_accept(ssl) <= 0)
        {
            close();
            throw std::runtime_error("SSL accept error");
        }
        */
        
        fcntl(fd, F_SETFL, O_NONBLOCK);
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
    
    std::pair <size_t, bool> write (std::span <const char> buffer)
    {
        check_error();
        int ret = SSL_write(ssl, buffer.data(), buffer.size());
        if (ret <= 0)
        {
            if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_WRITE)
                return {0, 0};
            throw std::runtime_error("ssl error write");
        }
        return {ret, 0};
    }
    
    void send_file (file_to_send_t & file)
    {
        check_error();
        if (BIO_get_ktls_send(SSL_get_wbio(ssl)))
        {
            // std::cerr << "ktls" << std::endl;
            // TODO enable this
            // modprobe tls
            ssize_t ret = SSL_sendfile(ssl, file.fd, file.offset, file.size - file.offset, 0);
            if (ret < 0)
            {
                if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_WRITE)
                    return;
                throw std::runtime_error("ssl error send file");
            }
            file.offset += ret;
        }
        else
        {
            // std::cerr << "no ktls" << std::endl;
            char buffer[20 * 1024];
            ssize_t rb = ::read(file.fd, buffer, sizeof(buffer));
            if (rb == -1)
            {
                if (errno == EINTR ||
                    errno == EAGAIN)
                    throw std::runtime_error("ssl error read file");
                return;
            }
            size_t wb = write(std::span <char> (buffer, rb)).first;
            file.offset += wb;
            if ((size_t)rb != wb)
            {
                off_t ret = lseek(file.fd, file.offset, SEEK_SET);
                if (ret == -1)
                    throw std::runtime_error("ssl error seek file");
            }
        }
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
    
    void do_accept ()
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
    
    std::optional <zero_copy_range_t> notify_err ()
    {
        return std::nullopt;
    }
    
    int fd;
    SSL * ssl;
    bool need_read;
    bool need_write;
};


#endif

