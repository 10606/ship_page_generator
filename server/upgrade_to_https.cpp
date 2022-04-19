#include "upgrade_to_https.h"

#include <stdexcept>
#include <iostream>


std::string_view get_host_port (mg_http_message * http_msg)
{
    for (size_t i = 0; i != 40; ++i)
    {
        if (std::string_view(http_msg->headers[i].name.ptr, http_msg->headers[i].name.len) == "Host")
            return std::string_view(http_msg->headers[i].value.ptr, http_msg->headers[i].value.len);
    }
    throw std::runtime_error("can't find host");
}

std::string_view get_host (mg_http_message * http_msg)
{
    std::string_view host_port = get_host_port(http_msg);
    size_t pos = host_port.rfind(':');
    return host_port.substr(0, pos);
}

std::string url (mg_http_message * http_msg, std::string const & port)
{
    return "https://" +
           std::string(get_host(http_msg)) + ":" + port +
           std::string(http_msg->uri.ptr, http_msg->uri.len) + "?" +
           std::string(http_msg->query.ptr, http_msg->query.len);
}

bool upgrade_if_need (mg_connection * nc, mg_http_message * http_msg, std::string const & port) noexcept
{
    if (nc->is_tls)
        return 0;
    for (size_t i = 0; i != 40; ++i)
    {
        if ((std::string_view(http_msg->headers[i].name.ptr,  http_msg->headers[i].name.len) == "Upgrade-Insecure-Requests") &&
            (std::string_view(http_msg->headers[i].value.ptr, http_msg->headers[i].value.len) == "1"))
        {
            try
            {
                std::string url_str = url(http_msg, port);
                mg_printf(nc, "HTTP/1.1 307 Temporary Redirect\r\n"
                          "Location: %s\r\n"
                          "\r\n", url_str.c_str());
                mg_iobuf_del(&nc->recv, 0, nc->recv.len);
                nc->recv.len = 0;
                nc->is_draining = 1;
                return 1;
            }
            catch (...)
            {}
        }
    }
    return 0;
}

