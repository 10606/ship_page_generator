#ifndef THROWERS_H
#define THROWERS_H

#include <vector>
#include <unordered_map>
#include <chrono>
#include <cmath>
#include "allocator.h"
#include "simple_string.h"
#include "ship_requests.h"
#include "armament_info.h"
#include "ship_armament_lt.h"


struct ship_throwers
{
    ship_throwers (ship_requests & database, std::string_view _new_line);

    struct responses_common_t
    {
        static const constexpr int group = 0;
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
    
        number_holder <uint32_t> data_begin;
        std::string_view data_end;
    };

    typedef ship_requests::ship_armament_lt_t::throwers ship_throwers_t;
    typedef ship_requests::armament_info_t::throwers throwers_t;
    
    std::vector <response_t, allocator_for_temp <response_t> >
    response (int id, std::chrono::year_month_day date) const;
    
    p_response_t partial_response (throwers_t const & thrower);

    struct ship_items_lt
    {
        ship_items_lt (size_t _thrower_id, ship_throwers_t const & value) :
            thrower_id(_thrower_id),
            mount_count(value.mount_count),
            date_from  (value.date_from),
            date_to    (value.date_to)
        {}
        
        size_t thrower_id;
        uint32_t mount_count;
        std::optional <std::chrono::year_month_day> date_from;
        std::optional <std::chrono::year_month_day> date_to;
    };
    
private:
    std::unordered_map <int, std::vector <ship_items_lt> > ship_throwers_list;
    std::vector <p_response_t> throwers;
    
    std::string new_line;
    std::string group_name;
};


#endif

