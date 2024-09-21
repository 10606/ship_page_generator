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

struct responser
{
    responser (ship_requests & database) :
        ship_list(&database),
        resp()
    {}
    
    struct resp_base
    {
        virtual void response (simple_string &, std::string_view, piece_t) = 0;
        virtual ~resp_base () = default;
        virtual std::string_view additional_in_menu () const = 0;
    };
    
    template <typename T>
    T & get (std::string_view uri);
    
    template <typename T>
    T & get_unsafe (std::string_view uri);

    template <typename T>
    struct resp_impl : resp_base
    {
        template <typename ... U>
        resp_impl (U && ... args) :
            value(std::forward <U> (args) ...)
        {}

        virtual void response (simple_string & answer, std::string_view query, piece_t title) override
        {
            value.response(answer, query, title);
        }

        virtual std::string_view additional_in_menu () const override
        {
            return additional_in_menu <T> (value);
        }

        virtual ~resp_impl () = default;
    
        template <typename U>
        friend U & responser::get (std::string_view uri);
        
        template <typename U>
        friend U & responser::get_unsafe (std::string_view uri);
        
    private:
        template <typename U>
        static std::string_view additional_in_menu (U const &)
        {
            return {};
        }
        
        template <typename U>
        requires requires (U const & value)
        {
            {value.menu_list} -> std::convertible_to <std::string_view>;
        }
        static std::string_view additional_in_menu (U const & value)
        {
            return value.menu_list;
        }
        
        T value;
    };

    
    template <typename T, typename ... U>
    void reg (std::string_view name, U && ... args)
    {
        resp.insert({name, std::make_unique <resp_impl <T> > (std::forward <U> (args) ...)});
    }
    
    bool response (simple_string & answer, std::string_view uri, std::string_view query)
    {
        std::unordered_map <std::string_view, std::unique_ptr <resp_base> > :: iterator it =
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
    std::unordered_map <std::string_view, std::unique_ptr <resp_base> > resp;
};

template <typename T>
T & responser::get (std::string_view uri)
{
    std::unordered_map <std::string_view, std::unique_ptr <resp_base> > :: iterator it =
        resp.find(uri);
    if (it == resp.end())
        throw std::runtime_error("can't find responser");
    resp_impl <T> & value_impl = dynamic_cast <resp_impl <T> &> (*it->second);
    return value_impl.value;
}

template <typename T>
T & responser::get_unsafe (std::string_view uri)
{
    resp_impl <T> & value_impl = static_cast <resp_impl <T> &> (*resp.find(uri)->second);
    return value_impl.value;
}


#endif

