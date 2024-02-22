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

static const constexpr html_template_3 style = 
{
"<!DOCTYPE html> \n"
"<html> \n"
    "<head> \n"
        "<title> \n",

        "</title> \n"
    "</head> \n"
    "<body> \n"
        "<style> \n"
            "* { font-size: 16px; } \n"
            ".wrapper { display: flex; } \n"
            "footer { \n"
                "display: table; \n"
                "color: #111111; \n"
                "font-size: 10px; \n"
                "text-align: center; \n"
                "margin-left: auto; \n"
                "margin-right: auto; \n"
                "border-top: 2px solid black;"
            "} \n"

            ".menu { \n"
                "display:    inline; \n"
                "min-width:  170pt; \n"
                "width:      170pt; \n"
                "max-width:  170pt; \n"
                "padding: 0px 10px 0px 0px; \n"
            "} \n"
            ".main { display: inline; } \n"
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
            ".main ul { \n"
                "list-style-type: disc; \n"
            "} \n"

            ".collapsible { \n"
                "background-color: #777777; \n"
                "color: white; \n"
                "cursor: crosshair; \n"
                "padding: 6px; \n"
                "border: none; \n"
                "text-align: left; \n"
                "outline: none; \n"
                "width: 100%; \n"
            "} \n"
            ".collapsible a { \n"
                "color: #ffffff; \n"
            "} \n"
            ".active { background-color: #555555; } \n"
            ".collapsible:hover { background-color: #ff5555; } \n"
            ".collapsible:focus { background-color: #6666ff; } \n"
            ".collapsible > a:focus { color: #aaeeff; } \n"
            ".content { \n"
                "padding: 0px 0px 0px 20px; \n"
                "display: none; \n"
                "overflow: hidden; \n"
            "} \n"
            ".content > a { \n"
                "color: #000000; \n"
                "background-color: #ffffff; \n"
            "} \n"
            ".content > a:hover { color: #ff2222; } \n"
            ".content > a:focus { color: #2222ff; } \n"
            ".menu_link > a { \n"
                "color: #000000; \n"
                "background-color: #ffffff; \n"
            "} \n"
            ".menu_link > a:hover { color: #ff2222; } \n"
            ".menu_link > a:focus { color: #2222ff; } \n"

            ".ship_armament tr { white-space: nowrap; } \n"
            ".ship_armament tr.header    { background: #fff8dc; } \n"
            ".ship_armament tr.general   { background: #f8ffdc; } \n"
            ".ship_armament tr.guns      { background: #cdffcc; } \n"
            ".ship_armament tr.torpedo   { background: #ffdfdf; } \n"
            ".ship_armament tr.throwers  { background: #f8ffdc; } \n"
            ".ship_armament tr.searchers { background: #f8ffff; } \n"
            ".ship_armament tr.catapult  { background: #fff8dc; } \n"
            ".ship_armament tr.aircraft  { background: #dbffff; } \n"
            ".ship_armament tr.propulsion{ background: #dfdfdf; } \n"
            ".ship_armament th { min-width: 180px } \n"
            
            ".armament { table-layout: fixed; } \n"
            ".armament td { \n"
                "min-width: 150px; \n"
                "max-width: 150px; \n"
            "} \n"
            
            ".short_info th      { min-width: 120px; } \n"
            ".short_info a       { color: #222222; } \n"
            ".short_info a:hover { color: #ff2222; } \n"
            ".short_info a:focus { color: #2222ff; } \n"
            
            "input { width: 97%; margin: 10px 20px 20px 0; }\n"
            "input:focus { outline-color: #2222ff; } \n"
            "h2 { display: inline; } \n"
            ".events { font-size: 16px; } \n"
            ".events * { font-size: 16px; } \n"
            ".counter { float:right; } \n"
            ".document       { min-width: 0px; } \n"
            ".document_group { min-width: 1200px; } \n"
        "</style> \n"
        "<div class = \"wrapper\"> \n",

        "</div> \n"
        "<footer> \n"
            "disclaimer: \n"
            "мир перестал существовать 02.09.1945<br> \n"
            "и да, здесь не просто могут быть... а гарантирванно есть ошибки и неточности!<br> \n"
            "сайт в разработке и хостится даже не в гараже (нету у меня гаража), поэтому SLA 9.999%<br> \n"
        "</footer> \n"
    "</body> \n"
"</html>"
};

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

        virtual ~resp_impl () = default;
    
        template <typename U>
        friend U & responser::get (std::string_view uri);
        
        template <typename U>
        friend U & responser::get_unsafe (std::string_view uri);
        
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

        answer.reserve(150000);

        answer.append(style.begin);
        size_t title_pos = answer.size();
        static const std::string title_placeholder(100, ' ');
        answer.append(title_placeholder);
        
        answer.append(style.middle);
        ship_list.response(answer, query);
        answer.append("<div class = \"main\">\n");
        it->second->response(answer, query, {title_pos, title_placeholder.size()});
        answer.append("</div>\n");
        answer.append(style.end);
        
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

template <typename T>
T & responser::get_unsafe (std::string_view uri)
{
    resp_impl <T> & value_impl = static_cast <resp_impl <T> &> (*resp.find(std::string(uri))->second);
    return value_impl.value;
}


#endif

