#define MG_ENABLE_SSL true
#define MG_ENABLE_CALLBACK_USERDATA true
#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include "mongoose.h"

#include "validate_path.h"
#include "upgrade_to_https.h"
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


struct http_addresses
{
    std::string http;
    std::string https;
};

struct https_server
{
    https_server 
    (
        std::initializer_list <http_addresses> addr_list
    ) :
        database(std::in_place),
        resp(*database),
        http_mark{this, 0},
        https_mark{this, 1}
    {
        mg_mgr_init(&mgr);
        
        for (http_addresses const & addr : addr_list)
        {
            mg_http_listen(&mgr, addr.http.c_str(),  ev_handler, &http_mark);
            mg_http_listen(&mgr, addr.https.c_str(), ev_handler, &https_mark);
        }
        
        mg_mgr_poll(&mgr, 100);
        mg_log_set("0");

        
        resp.reg <ship_armament>    ("/ship/armament",          &(*database));
        resp.reg <torpedo>          ("/armament/torpedo",       &(*database));
        resp.reg <guns>             ("/armament/guns",          &(*database));
        resp.reg <torpedo_tubes>    ("/armament/torpedo_tubes", &(*database));
        resp.reg <mines_charges>    ("/armament/mines_charges", &(*database));
        resp.reg <catapult>         ("/armament/catapult",      &(*database));
        resp.reg <searcher>         ("/armament/searcher",      &(*database));
        resp.reg <aircraft>         ("/aircraft",               &(*database));
        resp.reg <ship>             ("/ship",                   &(*database), resp.get <ship_armament> ("/ship/armament"));
        resp.reg <search>           ("/search",                 &(*database));

        database.reset();
    }

    static const char * s_ssl_cert;
    static const char * s_ssl_key;
    
    static void send_file (mg_connection * nc, int ev, void * ev_data, char const * path)
    {
        mg_http_message * http_msg = reinterpret_cast <mg_http_message *> (ev_data);
        mg_http_serve_opts opts = {.root_dir = "."};
        if (ev == MG_EV_HTTP_MSG)
            mg_http_serve_file(nc, http_msg, path, &opts);
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
        
        if (ev == MG_EV_HTTP_MSG)
        {
            mg_http_message * http_msg = reinterpret_cast <mg_http_message *> (ev_data);
            
            uint32_t code;
            static const std::string_view http_begin = "HTTP/1.1 ";
            static const std::string_view http_middle = "\r\nServer: japan_ships\r\n"
                                                        "Content-Type: text/html; charset=utf-8\r\n"
                                                        "Content-Length: ";
            static const std::string code_padding(' ', get_resp_code_str.max_size());
            static const std::string length_padding(' ', std::numeric_limits<size_t>::digits10 + 1);
            
            simple_string response;
            response.append(http_begin)
                    .append(code_padding)
                    .append(http_middle)
                    .append(length_padding)
                    .append("\r\nConnection: close\r\n\r\n");
            size_t http_header_size = response.size();
            
            if (std::string_view(http_msg->method.ptr, http_msg->method.len) != "GET")
            {
                response.append("??");
                code = 405;
            }
            else if (upgrade_if_need(nc, http_msg, "8443"))
                return;
            else
            {
                std::string_view uri(http_msg->uri.ptr, http_msg->uri.len);
                bool answer = 
                    cur->resp.response
                    (
                        response,
                        uri,
                        std::string_view(http_msg->query.ptr, http_msg->query.len)
                    );
                if (answer)
                {
                    code = 200;
                }
                else
                {
                    std::string path = filesystem_check(uri);
                    if (!path.empty())
                    {
                        send_file(nc, ev, ev_data, path.c_str());
                        return;
                    }
                    else
                        code = 403;
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
            
            response.rewrite(http_begin.size(), get_resp_code_str(code).data());
            std::string length = std::to_string(response.size() - http_header_size);
            response.rewrite(http_begin.size() +
                             code_padding.size() +
                             http_middle.size(), length);
            size_t answer_size = response.size();
            mg_send__eat_buf(nc, response.reset(), answer_size);
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
    std::optional <ship_requests> database;
    responser resp;
    
    struct mg_mgr mgr;
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
        https_server server{{"http://[::]:8080", "https://[::]:8443"}};

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

