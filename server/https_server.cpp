#define MG_ENABLE_SSL true
#define MG_ENABLE_CALLBACK_USERDATA true
#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <mongoose.h>

#include "upgrade_to_https.h"
#include "response_ship_armament.h"
#include "response_torpedo.h"
#include "response_guns.h"
#include "response_torpedo_tubes.h"
#include "response_mines_charges.h"
#include "response_catapult.h"
#include "response_searchers.h"
#include "response.h"


std::string_view get_resp_code_str (uint32_t code)
{
    static const std::map <uint32_t, std::string_view> answer = 
    {
        {200, "OK"},
        {403, "Forbidden"},
        {404, "Not found"},
        {405, "Method not allowed"},
    };
    
    std::map <uint32_t, std::string_view> ::const_iterator it = answer.find(code);
    if (it == answer.end())
        return "";
    return it->second;
}


struct https_server
{
    https_server 
    (
        std::atomic <bool> const & _stop, 
        std::string const & http_port,
        std::string const & https_port,
        std::string const & _response_value
    ) :
        database(),
        resp(&database),
        stop(_stop),
        response_value(_response_value)
    {
        mg_mgr_init(&mgr, NULL);
        memset(&bind_opts, 0, sizeof(bind_opts));
        bind_opts.user_data = this;
        bind_opts.error_string = &err;

        nc_http = mg_bind_opt(&mgr, http_port.c_str(), ev_handler, this, bind_opts);
        if (nc_http == NULL)
            throw std::runtime_error("can't bind");
        mg_set_protocol_http_websocket(nc_http);

        bind_opts.ssl_cert = s_ssl_cert;
        bind_opts.ssl_key = s_ssl_key;

        nc_https = mg_bind_opt(&mgr, https_port.c_str(), ev_handler, this, bind_opts);
        if (nc_https == NULL)
            throw std::runtime_error("can't bind");
        mg_set_protocol_http_websocket(nc_https);

        mg_mgr_poll(&mgr, 100);

        
        resp.reg <ship_armament>    ("/ship/armament",          &database);
        resp.reg <torpedo>          ("/armament/torpedo",       &database);
        resp.reg <guns>             ("/armament/guns",          &database);
        resp.reg <torpedo_tubes>    ("/armament/torpedo_tubes", &database);
        resp.reg <mines_charges>    ("/armament/mines_charges", &database);
        resp.reg <catapult>         ("/armament/catapult",      &database);
        resp.reg <searcher>         ("/armament/searcher",      &database);
    }

    static const char * s_ssl_cert;
    static const char * s_ssl_key;

    static void ev_handler (mg_connection * nc, int ev, void * ev_data, void *)
    {
        if (ev == MG_EV_HTTP_REQUEST)
        {
            mbuf * io = &nc->recv_mbuf;
            http_message * http_msg = reinterpret_cast <http_message *> (ev_data);
            https_server * cur = reinterpret_cast <https_server *> (nc->user_data);
            
            uint32_t code;
            std::string response;
            
            if (std::string_view(http_msg->method.p, http_msg->method.len) != "GET")
            {
                response = "??";
                code = 405;
            }
            else if (upgrade_if_need(nc, http_msg, "8443"))
                return;
            else
            {
                std::optional <std::string> answer = 
                    cur->resp.response
                    (
                        std::string_view(http_msg->uri.p, http_msg->uri.len),
                        std::string_view(http_msg->query_string.p, http_msg->query_string.len)
                    );
                if (answer)
                {
                    response = std::move(*answer);
                    code = 200;
                }
                else
                {
                    response = "Not found";
                    code = 404;
                }
            }
            
            /*
            std::string request(io->buf, io->len);
            response = cur->response_value.append("\r\n")
                .append(http_msg->uri.p, http_msg->uri.len).append("\r\n") 
                .append(http_msg->query_string.p, http_msg->query_string.len).append("\r\n")
                .append(http_msg->body.p, http_msg->body.len).append("\r\n")
                .append(http_msg->message.p, http_msg->message.len);
            
            // history
            for (size_t i = 0; i != 40; ++i)
            {
                response.append(http_msg->header_names[i].p, http_msg->header_names[i].len).append(" ");
                response.append(http_msg->header_values[i].p, http_msg->header_values[i].len).append("\r\n");
            }
            */
            
            mg_printf(nc, "HTTP/1.1 %u %s\r\n"
                        "Server: japan_ships\r\n"
                        "Content-Type: text/html; charset=utf-8\r\n"
                        "Content-Length: %lu\r\n"
                        "Connection: close\r\n"
                        "\r\n", code, get_resp_code_str(code).data(), response.size());
            mg_send
            (
                nc, 
                response.c_str(), 
                response.size()
            );
            mbuf_remove(io, io->len);
            nc->flags |= MG_F_SEND_AND_CLOSE;
        }
    }


    void main () 
    {
        while (!stop.load()) 
        {
            mg_mgr_poll(&mgr, 100);
        }
    }

    ~https_server ()
    {
        mg_mgr_free(&mgr);
    }
    
private:
    ship_requests database;
    responser resp;
    
    std::atomic <bool> const & stop;
    std::string response_value;

    struct mg_mgr mgr;
    struct mg_connection * nc_http;
    struct mg_connection * nc_https;
    struct mg_bind_opts bind_opts;
    const char * err;
};

const char * https_server::s_ssl_cert = "server/keys/server.pem";
const char * https_server::s_ssl_key = "server/keys/server.key";

void start_server (https_server & server)
{
    server.main();
}

struct server_starter 
{
    server_starter (std::string const & http_port, std::string const & https_port, std::string const & response) : 
        stop_server(0),
        server
        (
            stop_server, 
            http_port, 
            https_port, 
            response
        ),
        t_server(start_server, std::ref(server))
    {}

    ~server_starter ()
    {
        stop_server.store(1);
        t_server.join();
    }

private:
    std::atomic <bool> stop_server;
    https_server server;
    std::thread t_server;
};

int main ()
{
    server_starter server("0.0.0.0:8080", "0.0.0.0:8443", "________");
    while (1)
    {
        sleep(10);
    };
}

