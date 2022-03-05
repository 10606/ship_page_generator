#include "upgrade_to_https.h"

#include <stdexcept>
#include <iostream>


std::string_view get_host_port (http_message * http_msg)
{
    for (size_t i = 0; i != 40; ++i)
    {
        if (std::string_view(http_msg->header_names[i].p, http_msg->header_names[i].len) == "Host")
            return std::string_view(http_msg->header_values[i].p, http_msg->header_values[i].len);
    }
    throw std::runtime_error("can't find host");
}

std::string_view get_host (http_message * http_msg)
{
    std::string_view host_port = get_host_port(http_msg);
    size_t pos = host_port.rfind(':');
    return host_port.substr(0, pos);
}

std::string url (http_message * http_msg, std::string const & port)
{
    return "https://" +
           std::string(get_host(http_msg)) + ":" + port +
           std::string(http_msg->uri.p, http_msg->uri.len) + "?" +
           std::string(http_msg->query_string.p, http_msg->query_string.len);
}

bool upgrade_if_need (mg_connection * nc, http_message * http_msg, std::string const & port) noexcept
{
    if (nc->flags & SOL_TLS)
        return 0;
    for (size_t i = 0; i != 40; ++i)
    {
        if ((std::string_view(http_msg->header_names[i].p, http_msg->header_names[i].len) == "Upgrade-Insecure-Requests") &&
            (std::string_view(http_msg->header_values[i].p, http_msg->header_values[i].len) == "1"))
        {
            try
            {
                std::string url_str = url(http_msg, port);
                mbuf * io = &nc->recv_mbuf;
                mg_printf(nc, "HTTP/1.1 307 Temporary Redirect\r\n"
                          "Location: %s\r\n"
                          "\r\n", url_str.c_str());
                mbuf_remove(io, io->len);
                nc->flags |= MG_F_SEND_AND_CLOSE;
                return 1;
            }
            catch (...)
            {}
        }
    }
    return 0;
}

