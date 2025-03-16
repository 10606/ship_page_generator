#ifndef TORPEDO_H
#define TORPEDO_H

#include <vector>
#include <unordered_map>
#include <chrono>
#include <cmath>
#include "allocator.h"
#include "ship_requests.h"
#include "armament_info.h"
#include "ship_armament_lt.h"
#include "simple_string.h"


struct ship_torpedo_tubes
{
    ship_torpedo_tubes (ship_requests & database, std::string_view _new_line);

    struct responses_common_t
    {
        static const constexpr bool group = 0;
        int compare;
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
    
        number_holder <uint32_t> data_begin;
        std::string_view data_end;
    };

    typedef ship_requests::ship_armament_lt_t::torpedo_tubes ship_tubes_t;
    typedef ship_requests::armament_info_t::torpedo_tubes tube_t;
    
    std::vector <response_t, allocator_for_temp <response_t> >
    response (int id, std::chrono::year_month_day date) const;
    
    p_response_t partial_response (tube_t const & tube);

    struct ship_items_lt
    {
        ship_items_lt (size_t _tube_id, ship_tubes_t const & value) :
            tube_id(_tube_id),
            mount_count(value.mount_count),
            date_from  (value.date_from),
            date_to    (value.date_to)
        {}
        
        size_t tube_id;
        uint32_t mount_count;
        std::optional <std::chrono::year_month_day> date_from;
        std::optional <std::chrono::year_month_day> date_to;
    };
    
private:
    std::unordered_map <int, std::vector <ship_items_lt> > ship_tubes_list;
    std::vector <p_response_t> torpedo_tubes;
    
    std::string new_line;
    std::string group_name;
};


#endif

