#ifndef PROPULSION_H
#define PROPULSION_H

#include <vector>
#include <chrono>
#include "ship_requests.h"
#include "ship_propulsion.h"


struct ship_propulsion
{
    ship_propulsion (ship_requests * database, std::string_view _new_line);

    struct p_response_t
    {
        bool group;
        bool compare;
        std::string_view group_name;
        std::string data;
    };

    struct response_t
    {
        response_t () = default;
    
        response_t (p_response_t const & value) :
            group(value.group),
            compare(value.compare),
            group_name(value.group_name),
            data_begin(),
            data_end(value.data)
        {}
    
        bool group;
        bool compare;
        std::string_view group_name;
        char data_begin[16];
        std::string_view data_end;
    };

    typedef std::unique_ptr <ship_requests::propulsion_t::propulsion> propulsion_t;
    typedef ship_requests::propulsion_t::ship_propulsion ship_propulsions_t;
    
    std::vector <response_t> response (int id, std::chrono::year_month_day date) const;
    p_response_t partial_response (propulsion_t const & propulsion, ship_requests::propulsion_t::context const & storage);

    struct ship_items_lt
    {
        ship_items_lt (size_t _propulsion_id, ship_propulsions_t const & value) :
            propulsion_id(_propulsion_id),
            count    (value.count),
            date_from(value.date_from),
            date_to  (value.date_to)
        {}
        
        size_t propulsion_id;
        uint32_t count;
        std::optional <std::chrono::year_month_day> date_from;
        std::optional <std::chrono::year_month_day> date_to;
    };
    
private:
    std::unordered_map <int, std::vector <ship_items_lt> > ship_propulsions_list;
    std::vector <p_response_t> propulsions;
    
    std::string new_line;
    std::string group_name;
};

#endif

