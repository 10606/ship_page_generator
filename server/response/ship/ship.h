#ifndef SHIP_H
#define SHIP_H

#include "ship_event.h"
#include "response_ship_armament.h"
#include <map>


struct ship
{
    ship (ship_requests * _database, ship_armament & _armament);
    
    typedef ship_requests::aircraft_info_t::list aircraft_t;
 
    // http://127.0.0.1:8080/ship?id=0&id=1&id=2&id=3
    void response (std::string & answer, std::string_view query);
    
private:
    ship_armament & armament;
    
    struct response_t
    {
        std::string begin;
        std::string armament_link;
        std::string end;
    };
    
    std::unordered_map <int, response_t> modernizations;
    
    struct html_template
    {
        std::string_view begin;
        std::string_view end;
    };
    
    friend struct add_event;
    
    friend void add_general_info
    (
        std::string & answer, 
        std::string & modernization_link, 
        ship_requests::ship_info_t::list const & info
    );
    
    static const constexpr std::string_view query_template = "ship=";
    static const constexpr html_template link = {"<a href=\"/ship/armament?", "\">вооружение</a>"};
    static const constexpr std::string_view new_line = "<br>\n";
    static const constexpr std::string_view shift = "&emsp;";
};



#endif

