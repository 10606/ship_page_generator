#include <signal.h>
#include <sched.h>
#include "driver/server.h"
#include "validate_path.h"
#include "response_ship_armament.h"
#include "response_torpedo.h"
#include "response_guns.h"
#include "response_torpedo_tubes.h"
#include "response_mines_charges.h"
#include "response_catapult.h"
#include "response_searchers.h"
#include "response_aircraft.h"
#include "ship.h"
#include "response.h"
#include "simple_string.h"
#include "search.h"
#include "documents.h"
#include "file_cacher.h"
#include "day_events.h"


struct get_resp_code_str_t
{
    std::string_view operator () (uint32_t code)
    {
        
        std::map <uint32_t, std::string_view> ::const_iterator it = answer.find(code);
        if (it == answer.end())
            return "";
        return it->second;
    }
    
    size_t max_size ()
    {
        size_t max = 0;
        for (auto const & value : answer)
            max = std::max(max, value.second.size());
        return max;
    }
    
    static const std::map <uint32_t, std::string_view> answer;
} get_resp_code_str;

const std::map <uint32_t, std::string_view> get_resp_code_str_t::answer =
{
    {200, "200 OK"},
    {403, "403 Forbidden"},
    {404, "404 Not found"},
    {405, "405 Method not allowed"},
};


struct server_handler;

struct connection_handler
{
    connection_handler (server_handler * _cur) :
        cur(_cur)
    {}

    using headers_t = std::vector <std::pair <std::string_view, std::string_view> >;
    
    template <typename socket_t>
    void handle_head
    (
        connection <connection_handler, socket_t> & conn,
        std::string_view method,
        std::string_view uri,
        headers_t const & headers
    );
    
    template <typename socket_t>
    void read
    (
        connection <connection_handler, socket_t> & conn,
        std::string_view data
    )
    {}
    
    template <typename socket_t>
    void end_read
    (
        connection <connection_handler, socket_t> & conn
    )
    {}

    std::string_view get_host (std::string_view host_port)
    {
        // 127.0.0.1
        // 127.0.0.1:8080
        // [::1]:80
        size_t pos = host_port.rfind(':');
        if (pos >= host_port.size())
            return host_port;
        size_t i = pos + 1;
        for (; i != host_port.size(); ++i)
            if (!std::isdigit(host_port[i]))
                break;
        for (; i != host_port.size(); ++i)
            if (!std::isspace(host_port[i]))
                return host_port;
        return host_port.substr(0, pos);
    }

    std::string url (std::string_view uri_full, std::string_view host_port, std::string_view port)
    {
        std::string answer("https://");
        answer.append(get_host(host_port))
              .append(":")
              .append(port)
              .append(uri_full);
        return answer;
    }

    template <typename socket_t>
    bool upgrade_if_need
    (
        connection <connection_handler, socket_t> & conn,
        std::string_view uri_full, 
        headers_t const & headers, 
        std::string_view port
    ) noexcept
    {
        if constexpr (std::is_same_v <socket_t, ssl_socket>)
            return 0;
        bool need_upgrade = 0;
        std::optional <std::string_view> host_port;
        for (auto header : headers)
        {
            if (header.first == "Upgrade-Insecure-Requests" &&
                header.second == "1")
                need_upgrade = 1;
            if (header.first == "Host")
                host_port = header.second;
        }
        if (need_upgrade && host_port)
        {
            std::string url_str("HTTP/1.1 307 Temporary Redirect\r\n");
            url_str.append("Location: ")
                   .append(url(uri_full, *host_port, port))
                   .append("\r\n\r\n");
            conn.send(url_str);
            return 1;
        }
        return 0;
    }
        
private:
    server_handler * cur;
};

struct server_handler
{
    server_handler () :
        database(std::in_place),
        resp(*database),
        ship_names(*database)
    {
        resp.reg <ship_armament>    ("/ship/armament",          *database);
        resp.reg <torpedo>          ("/armament/torpedo",       *database);
        resp.reg <guns>             ("/armament/guns",          *database);
        resp.reg <torpedo_tubes>    ("/armament/torpedo_tubes", *database);
        resp.reg <mines_charges>    ("/armament/mines_charges", *database);
        resp.reg <catapult>         ("/armament/catapult",      *database);
        resp.reg <searcher>         ("/armament/searcher",      *database);
        resp.reg <aircraft>         ("/aircraft",               *database);
        resp.reg <ship>             ("/ship",                   *database, resp.get_unsafe <ship_armament> ("/ship/armament"));
        resp.reg <search>           ("/search",                 *database, ship_names);
        resp.reg <document>         ("/documents",              *database);
        resp.reg <day_events>       ("/",                       *database, ship_names);

        database.reset();
        
        file_cache.add_file("/pictures/favicon.ico");
    }
    
    server_handler (server_handler &&) = delete;
    server_handler (server_handler const &) = delete;
    server_handler & operator = (server_handler &&) = delete;
    server_handler & operator = (server_handler const &) = delete;

    connection_handler accept ()
    {
        return {this};
    }

    std::optional <ship_requests> database;
    responser resp;
    file_cacher file_cache;
    ship_names_list ship_names;
};

template <typename socket_t>
void connection_handler::handle_head
(
    connection <connection_handler, socket_t> & conn,
    std::string_view method,
    std::string_view uri_full,
    std::vector <std::pair <std::string_view, std::string_view> > const & headers
)
{
    uint32_t code;
    static const std::string_view http_begin = "HTTP/1.1 ";
    static const std::string code_padding(get_resp_code_str.max_size(), ' ');
    static const std::string_view http_middle = "\r\n"
                                                "Content-Type: text/html; charset=utf-8\r\n"
                                                "Content-Length: ";
    static const constexpr std::string_view unused_header = "\r\nServer: japan_ships";
    // some stupid clients not allow spaces at end of Content-Length header, so we place something after it
    static const constexpr size_t length_size = std::numeric_limits <size_t> ::digits10 + 1;
    static const std::string length_padding(length_size + unused_header.size(), ' ');
    
    simple_string response;
    response.append
    (
        http_begin,
        code_padding,
        http_middle,
        length_padding
    );
    if (conn.get_keep_alive())
        response.append("\r\nConnection: keep-alive\r\n\r\n");
    else
        response.append("\r\nConnection: close\r\n\r\n");
    size_t http_header_size = response.size();
    
    auto print_request = [method, uri_full] () -> void
    {
        try
        {
            std::cerr << "\033[01;36mmethod\033[0m " << method << " \033[01;36murl\033[0m " << uri_full << std::endl;
        }
        catch (...)
        {}
    };
    
    if (method != "GET")
    {
        response.append("??");
        code = 405;
        print_request();
    }
    else if (upgrade_if_need <socket_t> (conn, uri_full, headers, "8443"))
        return;
    else
    {
        size_t query_pos = uri_full.find('?');
        std::string_view uri = uri_full.substr(0, query_pos);
        std::string_view query = (query_pos == std::string_view::npos)? 
                                 std::string_view() : 
                                 uri_full.substr(query_pos + 1);
        bool answer = 
            cur->resp.response
            (
                response,
                uri,
                query
            );
        if (answer)
            code = 200;
        else
        {
            if (cur->file_cache.response(response, uri, headers))
            {
                size_t answer_size = response.size();
                conn.send(response.reset(), answer_size);
                return;
            }
            std::string path = filesystem_check(uri);
            if (!path.empty())
            {
                conn.send_file(path);
                return;
            }
            else
            {
                code = 403;
                print_request();
            }
        }
    }
    
    // status code
    response.rewrite(http_begin.size(), get_resp_code_str(code).data());
    // content length
    char * length_place = response.data() +
                          http_begin.size() +   
                          code_padding.size() +
                          http_middle.size();
    std::to_chars_result length_convert = std::to_chars(length_place, length_place + length_size, response.size() - http_header_size);
    // placeholder after content length
    response.rewrite(http_begin.size() +
                     code_padding.size() +
                     http_middle.size() +
                     length_convert.ptr - length_place,
                     unused_header);
    size_t answer_size = response.size();
    conn.send(response.reset(), answer_size);
    if (code == 405)
        conn.end_read(0);
}

volatile bool run = 1;

void handler_exit (int)
{
    run = 0;
}

void set_sig_handler (int sig_num, void (* handler) (int))
{
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    int ret = sigaction(sig_num, &act, NULL);
    if (ret)
        std::runtime_error("can't set signal handler");
}

int main ()
{
    try
    {
        set_sig_handler(SIGTERM, handler_exit);
        set_sig_handler(SIGINT,  handler_exit);
        set_sig_handler(SIGQUIT, handler_exit);
        set_sig_handler(SIGPIPE, SIG_IGN);
        set_sig_handler(SIGUSR1, SIG_IGN);
        set_sig_handler(SIGUSR2, SIG_IGN);
        
        cpu_set_t cpu_mask;
        CPU_ZERO(&cpu_mask);
        CPU_SET(0, &cpu_mask);
        CPU_SET(2, &cpu_mask);
        sched_setaffinity(0, sizeof(cpu_mask), &cpu_mask);

        std::string_view ssl_cert = "server/keys/server.pem";
        std::string_view ssl_key = "server/keys/server.key";
        
        server <server_handler> http_server
        (
            {
                {8080, 0},
                {8443, 1}
            },
            ssl_cert,
            ssl_key
        );
        
        while (run)
            http_server.process(100);
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what();
    }
    catch (...)
    {}
}

