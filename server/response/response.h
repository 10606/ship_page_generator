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
    responser (ship_requests * _database) :
        database(_database),
        ship_list(database),
        resp()
    {}

    struct resp_base
    {
        virtual std::string response (std::string_view) = 0;
        virtual ~resp_base () = default;
    };
    
    template <typename T>
    struct resp_impl : resp_base
    {
        template <typename ... U>
        resp_impl (U && ... args) :
            value(std::forward <U> (args) ...)
        {}
        
        virtual std::string response (std::string_view query)
        {
            return value.response(query);
        }
        
        virtual ~resp_impl () = default;
    
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
        std::map <std::string, std::unique_ptr <resp_base> > :: iterator it =
            resp.find(std::string(uri));
        if (it == resp.end())
            return std::nullopt;

        std::string answer;

        answer += "<style> \n\
                      .wrapper { display: flex; } \n\
                   </style>\n";
        answer += "<div class = \"wrapper\"> \n\
                   <style> \n\
                        .menu { display: inline; } \n\
                        .main { display: inline; } \n\
                   </style>\n";
        answer += 
            ship_list.response() +
            "<div class = \"main\">\n" + 
            it->second->response(query) +
            "</div>\n";
        answer += "</div>";
        
        return answer;
    }

private:
    ship_requests * database;
    menu ship_list;
    std::map <std::string, std::unique_ptr <resp_base> > resp;
};


#endif

