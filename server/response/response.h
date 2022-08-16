#ifndef RESPONSE_RESPONSE_H
#define RESPONSE_RESPONSE_H

#include <map>
#include <string>
#include <memory>
#include <optional>
#include "menu.h"
#include "ship_requests.h"
#include "simple_string.h"

static const constexpr std::string_view style = 
"<div class = \"wrapper\"> \n"
    "<style> \n"
        ".menu { \n"
            "display:    inline; \n"
            "min-width:  170pt; \n"
            "width:      170pt; \n"
            "max-width:  170pt; \n"
            "padding: 0px 10px 0px 0px; \n"
        "} \n"
        ".main { display:    inline; } \n"
        ".main img { \n"
            "object-fit: contain; \n"
            "min-width: 180px; \n"
            "max-width: 180px; \n"
            "min-height: 180px; \n"
            "max-height: 180px; \n"
            "background-color: #f5f5f5; \n"
        "} \n"
        ".main li { \n"
            "display: inline-block; \n"
            "margin: 10px; \n"
            "max-width: 180px; \n"
            "vertical-align: top; \n"
        "} \n"
    "</style>\n";

struct responser
{
    responser (ship_requests & database) :
        ship_list(&database),
        resp()
    {}
    
    struct resp_base
    {
        virtual void response (simple_string &, std::string_view) = 0;
        virtual ~resp_base () = default;
    };
    
    template <typename T>
    T & get (std::string_view uri);

    template <typename T>
    struct resp_impl : resp_base
    {
        template <typename ... U>
        resp_impl (U && ... args) :
            value(std::forward <U> (args) ...)
        {}
        
        virtual void response (simple_string & answer, std::string_view query)
        {
            value.response(answer, query);
        }

        virtual ~resp_impl () = default;
    
        template <typename U>
        friend U & responser::get (std::string_view uri);
        
    private:
        T value;
    };

    
    template <typename T, typename ... U>
    void reg (std::string name, U && ... args)
    {
        resp.insert({std::move(name), std::make_unique <resp_impl <T> > (std::forward <U> (args) ...)});
    }
    
    bool response (simple_string & answer, std::string_view uri, std::string_view query)
    {
        std::unordered_map <std::string, std::unique_ptr <resp_base> > :: iterator it =
            resp.find(std::string(uri));
        if (it == resp.end())
            return 0;

        answer.reserve(100000);

        answer.append("<style> \n"
                          ".wrapper { display: flex; } \n"
                      "</style>\n");
        answer.append(style);
        ship_list.response(answer);
        answer.append("<div class = \"main\">\n");
        it->second->response(answer, query);
        answer.append("</div>\n</div>");
        
        return 1;
    }

private:
    menu ship_list;
    std::unordered_map <std::string, std::unique_ptr <resp_base> > resp;
};

template <typename T>
T & responser::get (std::string_view uri)
{
    std::unordered_map <std::string, std::unique_ptr <resp_base> > :: iterator it =
        resp.find(std::string(uri));
    if (it == resp.end())
        throw std::runtime_error("can't find responser");
    resp_impl <T> & value_impl = dynamic_cast <resp_impl <T> &> (*it->second);
    return value_impl.value;
}


#endif

