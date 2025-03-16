#include "connection.h"
#include "raw_socket.h"
#include "ssl_socket.h"
#include "create_socket.h"

#include "driver/epoll_wrap.h"
#include <bits/chrono.h>
#include <chrono>


struct poll
{
    poll
    (
        std::pair <char const *, char const *> raw_addr,
        std::pair <char const *, char const *> ssl_addr,
        
        std::vector <std::string_view> request_uris,
        
        size_t _raw_connections,
        size_t _ssl_connections,
        
        size_t _requests_per_connection,
        size_t _queued_requests,
        size_t _total_requests
    ) :
        addresses_raw(raw_addr.first, raw_addr.second),
        addresses_ssl(ssl_addr.first, ssl_addr.second),
        raw_connections(_raw_connections),
        ssl_connections(_ssl_connections),
        
        requests_per_connection(_requests_per_connection),
        queued_requests(std::min(_queued_requests, _requests_per_connection)),
        total_requests(_total_requests)
    {
        /*
        requests.emplace_back("GET / HTTP/1.1\r\n"
                              "Connection: keep-alive\r\n"
                              "\r\n");
        */
        if (request_uris.empty())
            throw std::runtime_error("empty url list...");
        
        for (std::string_view request_uri : request_uris)
        {
            requests.emplace_back("GET ")
                    .append(request_uri)
                    .append(" HTTP/1.1\r\n"
                            "Connection: keep-alive\r\n"
                            "\r\n");
        }
        
        for (size_t i = 0; i != raw_connections; ++i)
            new_connect(0);
        for (size_t i = 0; i != ssl_connections; ++i)
            new_connect(1);
    }
    
    poll (poll &&) = delete;
    poll (poll const &) = delete;
    poll & operator = (poll &&) = delete;
    poll & operator = (poll const &) = delete;
 
    void close ()
    {
        for (auto const & conn : raw)
            close_connection(conn.second);
        for (auto const & conn : ssl)
            close_connection(conn.second);
        for (auto const & conn : connect)
            epoll.del(conn.second.fd);
        raw.clear();
        ssl.clear();
        connect.clear();
    }
    
    void process (int timeout = -1)
    {
        std::vector <epoll_wrap::envoke> events = epoll.wait(timeout);
        for (epoll_wrap::envoke event : events)
        {
            process_connect(event);
            process_main(ssl, event);
            process_main(raw, event);
        }
    }
    
    bool finish () const noexcept
    {
        bool finish = total_requests == 0;
        for (auto const & conn : raw)
            finish &= !conn.second.want_read() && !conn.second.want_write();
        for (auto const & conn : ssl)
            finish &= !conn.second.want_read() && !conn.second.want_write();
        return finish;
    }
    
    struct statistic
    {
        uint64_t complete_requests = 0;
        uint64_t read_bytes_body = 0;
        uint64_t read_bytes_total = 0;
    };
    
    statistic statistic_raw;
    statistic statistic_ssl;
    
private:
    template <typename T>
    void close_connection (connection <T> const & conn)
    {
        statistic & stat = std::is_same_v <T, raw_socket>? statistic_raw : statistic_ssl;
        
        stat.complete_requests += conn.complete_requests();
        stat.read_bytes_body   += conn.read_bytes_body;
        stat.read_bytes_total  += conn.read_bytes_total;

        epoll.del(conn.fd());
    }
    
    template <typename T>
    void process_main (std::map <int, connection <T> > & conn, epoll_wrap::envoke event)
    {
        typename std::map <int, connection <T> > ::iterator it = conn.find(event.fd);
        if (it == conn.end())
            return;

        auto close_conn = [this, &conn, it] () -> void
        {
            // update stat
            close_connection(it->second);
            conn.erase(it);
            new_connect(std::is_same_v <T, ssl_socket>);
        };
        
        if (event.events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR))
        {
            close_conn();
            return;
        }
        if (event.events & EPOLLIN)
            it->second.read();
        if (event.events & EPOLLOUT)
            it->second.write();
        while (it->second.can_read())
            it->second.read();
        while (it->second.can_process())
            it->second.process();
        add_requests(it->second);
        bool want_something = epoll_mod(it->second);
        if (!want_something && add_requests(it->second) == never_for_this)
            close_conn();
    }
    
    void process_connect (epoll_wrap::envoke event)
    {
        std::map <int, connect_socket> ::iterator it = connect.find(event.fd);
        if (it == connect.end())
            return;
        
        auto reconnect = [this, it, event] () -> void
        {
            epoll.del(event.fd);
            new_connect(it->second.is_ssl);
            connect.erase(it);
            connection_fails++;
        };
        
        if (event.events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR))
        {
            reconnect();
            return;
        }
        if (event.events & EPOLLOUT)
        {
            connect_socket::status_t status = it->second.check();
            switch (status)
            {
            case connect_socket::ok:
                it->second.move_fd();
                connect.erase(it);
                if (!it->second.is_ssl)
                {
                    std::map <int, connection <raw_socket> > ::iterator created =
                        raw.emplace(event.fd, event.fd).first;
                    add_requests(created->second);
                    epoll_mod(created->second);
                }
                else
                {
                    std::map <int, connection <ssl_socket> > ::iterator created =
                        ssl.emplace(event.fd, event.fd).first;
                    add_requests(created->second);
                    epoll_mod(created->second);
                }
                break;
            case connect_socket::error:
                reconnect();
                break;
            case connect_socket::again:
                break;
            }
        }
    }
    
    void new_connect (bool is_ssl)
    {
        get_addr & addresses = is_ssl? addresses_ssl : addresses_raw;
        addresses.reset();
        connect_socket socket(addresses, is_ssl);
        int fd = socket.fd;
        connect.insert({fd, std::move(socket)});
        try
        {
            epoll.add(fd, EPOLLOUT);
        }
        catch (...)
        {
            connect.erase(fd);
        }
    }
 
    template <typename T>
    bool epoll_mod (connection <T> const & conn)
    {
        uint32_t event_mask = 0;
        if (conn.want_read())
            event_mask |= EPOLLIN;
        if (conn.want_write())
            event_mask |= EPOLLOUT;
        epoll.mod(conn.fd(), event_mask);
        return event_mask != 0;
    };
    
    enum add_request_state
    {
        never,
        never_for_this,
        busy,
        ok
    };
    
    template <typename T>
    add_request_state add_requests (connection <T> & conn)
    {
        bool have_ok = 0;
        while (1)
        {
            if (!total_requests)
                return have_ok? ok : never;
            if (conn.get_sent() + conn.get_queued() > requests_per_connection)
                return have_ok? ok : never_for_this;
            if (conn.get_queued() > queued_requests)
                return have_ok? ok : busy;
            
            conn.send(requests[pos]);
            pos = (pos + 1) % requests.size();
            total_requests--;
            have_ok = 1;
        }
    }
    
    get_addr addresses_raw;
    get_addr addresses_ssl;

    std::map <int, connection <raw_socket> > raw;
    std::map <int, connection <ssl_socket> > ssl;
    std::map <int, connect_socket> connect;
    epoll_wrap epoll;
    
    size_t raw_connections;
    size_t ssl_connections;
    
    std::vector <std::string> requests;
    size_t pos = 0;

    size_t connection_fails = 0;
    size_t requests_per_connection;
    size_t queued_requests;
    size_t total_requests;
};

int main ()
{
    std::pair <const char *, const char *> raw_addr = {"127.0.0.1", "8080"};
    std::pair <const char *, const char *> ssl_addr = {"127.0.0.1", "8443"};

    std::vector <std::string_view> uris =
    {
        // "/documents", // static
        // "/../../.,/", // error
        // "asdkjasldkjaslkdjlkasjdlkasjdlkasjdlkasjdlkkajslkdjaslkdjaslkdjalskjdlaksjdlkasjdlkasjdlkasjdlkajsdlkjaslkdjasldjaslkdjaslkdjalkjdlkasjdlkasjdlkasjdlkasjdlkasjlkdasjlkdajslkdjaslkdjaslkdjaslkjdaslkjasldkjdlaskjdlaks/../../.,/", // error
        // "/favicon.ico", // file
        // "/", // day events
        "/ship?type_id=18",
        "/ship?type_id=21",
        "/ship?type_id=14",
        "/ship?type_id=15",
        "/ship?type_id=17",
        "/ship?type_id=18",
        "/ship?type_id=19",
        "/ship?type_id=3",
        "/search?search=terutsuki", // search
        "/search?search=makadze", // search
    };
    /*
    std::vector <std::string_view> uris =
    {
        "/ship?type_id=18",
        "/ship?type_id=21",
        "/ship?type_id=14",
        "/ship?type_id=15",
        "/ship?type_id=17",
        "/ship?type_id=18",
        "/ship?type_id=19",
        "/ship?type_id=3"
    };
    */
    
    size_t raw_connections = 20;
    size_t ssl_connections = 0;

    size_t requests_per_connection = 100;
    size_t queued_requests = 100;
    size_t total_requests = 100000;
    
    poll bench
    (
        raw_addr,
        ssl_addr,
        
        uris, 
        
        raw_connections,
        ssl_connections,
        
        requests_per_connection,
        queued_requests,
        total_requests
    );
    
    auto time_begin = std::chrono::high_resolution_clock::now();
    while (!bench.finish())
        bench.process();
    bench.close();
    auto time_end = std::chrono::high_resolution_clock::now();
    
    std::chrono::milliseconds spend = std::chrono::duration_cast <std::chrono::milliseconds> (time_end - time_begin);
    double spend_seconds = static_cast <double> (spend.count()) / 1000;
    std::cerr << spend_seconds << "s" << std::endl;
    std::cout << std::endl;

    std::cout << "raw:" << std::endl;
    std::cout << "  requests: " << bench.statistic_raw.complete_requests << std::endl;
    std::cout << "            " << static_cast <double> (bench.statistic_raw.complete_requests) / spend_seconds << " /s" << std::endl;
    std::cout << "  total:    " << bench.statistic_raw.read_bytes_total  << std::endl;
    std::cout << "            " << static_cast <double> (bench.statistic_raw.read_bytes_total)  / spend_seconds << " B/s" << std::endl;
    std::cout << "  body:     " << bench.statistic_raw.read_bytes_body   << std::endl;
    std::cout << "            " << static_cast <double> (bench.statistic_raw.read_bytes_body)   / spend_seconds << " B/s" << std::endl;
    std::cout << std::endl;
    
    std::cout << "ssl:" << std::endl;
    std::cout << "  requests: " << bench.statistic_ssl.complete_requests << std::endl;
    std::cout << "            " << static_cast <double> (bench.statistic_ssl.complete_requests) / spend_seconds << " /s" << std::endl;
    std::cout << "  total:    " << bench.statistic_ssl.read_bytes_total  << std::endl;
    std::cout << "            " << static_cast <double> (bench.statistic_ssl.read_bytes_total)  / spend_seconds << " B/s" << std::endl;
    std::cout << "  body:     " << bench.statistic_ssl.read_bytes_body   << std::endl;
    std::cout << "            " << static_cast <double> (bench.statistic_ssl.read_bytes_body)   / spend_seconds << " B/s" << std::endl;
    std::cout << std::endl;
}

