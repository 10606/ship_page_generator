#ifndef RESPONSE_RESPONSE_H
#define RESPONSE_RESPONSE_H

#include <map>
#include <string>
#include <memory>
#include <optional>
#include "menu.h"
#include "ship_requests.h"
#include "simple_string.h"
#include "html_template.h"
#include "parse_query.h"

extern const html_template_3 style;

struct response_base
{
    virtual ~response_base () = default;
    
    virtual void response (simple_string & answer, std::string_view query, piece_t title) = 0;
    
    virtual std::string_view additional_in_menu () const
    {
        return {};
    }
};

struct responser
{
    responser (ship_requests & database) :
        ship_list(&database),
        resp()
    {}
    
    template <typename T>
    T & get (std::string_view uri);
    
    template <typename T>
    T & get_unsafe (std::string_view uri);
    
    template <std::derived_from <response_base> T, typename ... U>
    void reg (std::string_view name, U && ... args)
    {
        resp.insert({name, std::make_unique <T> (std::forward <U> (args) ...)});
    }
    
    bool response (simple_string & answer, std::string_view uri, std::string_view query)
    {
        std::unordered_map <std::string_view, std::unique_ptr <response_base> > :: iterator it =
            resp.find(uri);
        if (it == resp.end())
            return 0;

        answer.reserve(150000);

        answer.append(style.begin);
        size_t title_pos = answer.size();
        static const std::string title_placeholder(100, ' ');
        answer.append(title_placeholder, style.middle);
        ship_list.response(answer, query, it->second->additional_in_menu());
        answer.append("<div class=\"main\">\n");
        it->second->response(answer, query, {title_pos, title_placeholder.size()});
        answer.append("</div>\n", style.end);
        
        return 1;
    }

private:
    menu ship_list;
    std::unordered_map <std::string_view, std::unique_ptr <response_base> > resp;
};

template <typename T>
T & responser::get (std::string_view uri)
{
    std::unordered_map <std::string_view, std::unique_ptr <response_base> > :: iterator it =
        resp.find(uri);
    if (it == resp.end())
        throw std::runtime_error("can't find responser");
    T & value_impl = dynamic_cast <T &> (*it->second);
    return value_impl;
}

template <typename T>
T & responser::get_unsafe (std::string_view uri)
{
    T & value_impl = static_cast <T &> (*resp.find(uri)->second);
    return value_impl;
}


#endif

