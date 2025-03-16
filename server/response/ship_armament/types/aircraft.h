#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <vector>
#include <chrono>
#include <cmath>
#include "allocator.h"
#include "ship_requests.h"
#include "armament_info.h"
#include "ship_armament_lt.h"
#include "date_to_str.h"


struct ship_aircrafts
{
    ship_aircrafts (ship_requests & database, std::string_view _new_line);

    struct responses_common_t
    {
        int group;
        static const constexpr bool compare = 0;
        std::string_view group_name;
    };
    
    struct p_response_t : responses_common_t
    {
        std::string data;
    };

    struct response_t : responses_common_t
    {
        response_t () = default;
    
        response_t (p_response_t const & value) :
            responses_common_t(value),
            data_begin(),
            data_end(value.data)
        {}
    
        std::string_view data_begin;
        std::string_view data_end;
    };

    typedef ship_requests::ship_armament_lt_t::aircraft ship_aircrafts_t;
    typedef ship_requests::aircraft_info_t::list aircraft_t;
    typedef ship_requests::aircraft_info_t::classes aircraft_class;
    
    std::vector <response_t, allocator_for_temp <response_t> >
    response (int id, std::chrono::year_month_day date) const;
    
    p_response_t partial_response (aircraft_t const & aircraft);

    struct ship_items_lt
    {
        ship_items_lt (size_t _aircraft_id, ship_aircrafts_t const & value) :
            aircraft_id(_aircraft_id),
            count(),
            date_from(value.date_from),
            date_to  (value.date_to)
        {
            add_value(count, value.count);
            if (value.count_reserve)
            {
                count.append("(+");
                add_value(count, value.count_reserve);
                count.append(")");
            }
        }
        
        size_t aircraft_id;
        std::string count;
        std::optional <std::chrono::year_month_day> date_from;
        std::optional <std::chrono::year_month_day> date_to;
    };
    
private:
    std::unordered_map <int, std::vector <ship_items_lt> > ship_aircrafts_list;
    std::vector <p_response_t> aircrafts;
    std::unordered_map <int, std::string> cache_class_names;
    
    std::string new_line;
};


#endif

