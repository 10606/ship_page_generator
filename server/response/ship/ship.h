#ifndef SHIP_H
#define SHIP_H

#include "ship_event.h"
#include "response_ship_armament.h"
#include "simple_string.h"
#include "html_template.h"
#include <map>


struct ship
{
    ship (ship_requests * _database, ship_armament & _armament);
    
    // http://127.0.0.1:8080/ship?id=0&id=1&id=2&id=3
    void response (simple_string & answer, std::string_view query);
    
private:
    ship_armament & armament;
    
    struct response_t
    {
        std::string short_info;

        std::string begin;
        std::string armament_link;
        std::string end;
    };
    
    std::unordered_map <int, response_t> modernizations;
    
    struct add_event;
    
    static void add_general_info
    (
        std::string & answer, 
        std::string & modernization_link, 
        ship_requests::ship_info_t::list const & info
    );

    static void add_short_info
    (
        std::string & answer, 
        ship_requests::ship_info_t::list const & info
    );
    
    static const constexpr std::string_view query_template = "ship=";
    static const constexpr html_template link = {"<a href=\"/ship/armament?", "\">вооружение</a>"};
    static const constexpr std::string_view new_line = "<br>\n";
    static const constexpr std::string_view shift = "&emsp;";
    
    static const constexpr pictures_template pictures =
    {
        {
            "<li><a href=\"/pictures/ship/",
            "\"><img src=\"/pictures/ship/",
            "\"></a><br>",
            "</li>"
        },
        {
            "<ul>",
            "</ul><br>"
        }
    };
};



#endif

