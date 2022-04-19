#define MG_ENABLE_SSL true
#define MG_ENABLE_CALLBACK_USERDATA true
#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include "mongoose.h"

#include "upgrade_to_https.h"
#include "response_ship_armament.h"
#include "response_torpedo.h"
#include "response_guns.h"
#include "response_torpedo_tubes.h"
#include "response_mines_charges.h"
#include "response_catapult.h"
#include "response_searchers.h"
#include "response_aircraft.h"
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
        std::string const & http_port,
        std::string const & https_port,
        std::string const & _response_value
    ) :
        database(),
        resp(&database),
        response_value(_response_value),
        http_mark{this, 0},
        https_mark{this, 1}
    {
        mg_mgr_init(&mgr);
        nc_http  = mg_http_listen(&mgr, http_port.c_str(),  ev_handler, &http_mark);
        nc_https = mg_http_listen(&mgr, https_port.c_str(), ev_handler, &https_mark);
        
        mg_mgr_poll(&mgr, 100);
        mg_log_set("0");

        
        resp.reg <ship_armament>    ("/ship/armament",          &database);
        resp.reg <torpedo>          ("/armament/torpedo",       &database);
        resp.reg <guns>             ("/armament/guns",          &database);
        resp.reg <torpedo_tubes>    ("/armament/torpedo_tubes", &database);
        resp.reg <mines_charges>    ("/armament/mines_charges", &database);
        resp.reg <catapult>         ("/armament/catapult",      &database);
        resp.reg <searcher>         ("/armament/searcher",      &database);
        resp.reg <aircraft>         ("/aircraft",               &database);
    }

    static const char * s_ssl_cert;
    static const char * s_ssl_key;
    
    static void fn (mg_connection * nc, int ev, void * ev_data, void *)
    {
        mg_http_message * http_msg = reinterpret_cast <mg_http_message *> (ev_data);
        mg_http_serve_opts opts = {.root_dir = "."};   // Serve local dir
        if (ev == MG_EV_HTTP_MSG) 
            mg_http_serve_dir(nc, http_msg, &opts);
    }

    static void ev_handler (mg_connection * nc, int ev, void * ev_data, void *)
    {
        std::pair <https_server *, bool> * mark = 
            reinterpret_cast <std::pair <https_server *, bool> *> (nc->fn_data);
        https_server * cur = mark->first;
        
        if (ev == MG_EV_ACCEPT && mark->second)
        {
            mg_tls_opts opts = 
            {
                .cert = s_ssl_cert,
                .certkey = s_ssl_key
            };
            mg_tls_init(nc, &opts);
            return;
        }
        //fn(nc, ev, ev_data, NULL);
        //return;
        
        if (ev == MG_EV_HTTP_MSG)
        {
            mg_http_message * http_msg = reinterpret_cast <mg_http_message *> (ev_data);
            
            uint32_t code;
            std::string response;
            
            if (std::string_view(http_msg->method.ptr, http_msg->method.len) != "GET")
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
                        std::string_view(http_msg->uri.ptr, http_msg->uri.len),
                        std::string_view(http_msg->query.ptr, http_msg->query.len)
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
            mbuf * io = &nc->recv_mbuf;
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
            mg_iobuf_del(&nc->recv, 0, nc->recv.len);
            nc->recv.len = 0;
            nc->is_draining = 1;
        }
    }


    void main () 
    {
        mg_mgr_poll(&mgr, 100);
    }

    ~https_server ()
    {
        mg_mgr_free(&mgr);
    }
    
private:
    ship_requests database;
    responser resp;
    
    std::string response_value;

    struct mg_mgr mgr;
    struct mg_connection * nc_http;
    struct mg_connection * nc_https;
    const char * err;
    std::pair <https_server *, bool> http_mark;
    std::pair <https_server *, bool> https_mark;
};

const char * https_server::s_ssl_cert = "server/keys/server.pem";
const char * https_server::s_ssl_key = "server/keys/server.key";


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
        set_sig_handler(SIGINT, handler_exit);
        https_server server("http://0.0.0.0:8080", "https://0.0.0.0:8443", "________");

        while (run)
            server.main();
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what();
    }
    catch (...)
    {}
}

