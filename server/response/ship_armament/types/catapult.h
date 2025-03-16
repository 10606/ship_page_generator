#ifndef CATAPULT_H
#define CATAPULT_H

#include <vector>
#include <chrono>
#include <cmath>
#include "allocator.h"
#include "simple_string.h"
#include "ship_requests.h"
#include "armament_info.h"
#include "ship_armament_lt.h"


struct ship_catapult
{
    ship_catapult (ship_requests & database, std::string_view _new_line);

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

    typedef ship_requests::armament_info_t::catapult catapult_t;
    typedef ship_requests::ship_armament_lt_t::catapult ship_catapults_t;
    
    std::vector <response_t, allocator_for_temp <response_t> >
    response (int id, std::chrono::year_month_day date) const;
    
    p_response_t partial_response (catapult_t const & catapult);

    struct ship_items_lt
    {
        ship_items_lt (size_t _catapult_id, ship_catapults_t const & value) :
            catapult_id(_catapult_id),
            count    (value.count),
            date_from(value.date_from),
            date_to  (value.date_to)
        {}
        
        size_t catapult_id;
        uint32_t count;
        std::optional <std::chrono::year_month_day> date_from;
        std::optional <std::chrono::year_month_day> date_to;
    };
    
private:
    std::unordered_map <int, std::vector <ship_items_lt> > ship_catapults_list;
    std::vector <p_response_t> catapults;
    
    std::string new_line;
    std::string group_name;
};


#endif

