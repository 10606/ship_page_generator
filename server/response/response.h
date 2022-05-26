#ifndef RESPONSE_RESPONSE_H
#define RESPONSE_RESPONSE_H

#include <map>
#include <string>
#include <memory>
#include <optional>
#include "menu.h"
#include "ship_requests.h"


struct responser
{
    responser (ship_requests & database) :
        ship_list(&database),
        resp()
    {}
    
    struct resp_base
    {
        virtual void response (std::string &, std::string_view) = 0;
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
        
        virtual void response (std::string & answer, std::string_view query)
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
    
    std::optional <std::string> response (std::string_view uri, std::string_view query)
    {
        std::unordered_map <std::string, std::unique_ptr <resp_base> > :: iterator it =
            resp.find(std::string(uri));
        if (it == resp.end())
            return std::nullopt;

        std::string answer;

        answer += "<style> \n\
                      .wrapper { display: flex; } \n\
                   </style>\n";
        answer += "<div class = \"wrapper\"> \n\
                   <style> \n\
                        .menu { \n\
                            display:    inline; \n\
                            min-width:  170pt; \n\
                            width:      170pt; \n\
                            max-width:  170pt; \n\
                            padding: 0px 10px 0px 0px; \n\
                        } \n\
                        .main { display:    inline; } \n\
                   </style>\n";
        ship_list.response(answer);
        answer.append("<div class = \"main\">\n");
        it->second->response(answer, query);
        answer += "</div>\n</div>";
        
        return answer;
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

