#ifndef SEARCHERS_H
#define SEARCHERS_H

#include <vector>
#include <unordered_map>
#include <chrono>
#include <cmath>
#include "allocator.h"
#include "simple_string.h"
#include "ship_requests.h"
#include "armament_info.h"
#include "ship_armament_lt.h"


struct ship_searchers
{
    ship_searchers (ship_requests * database, std::string_view _new_line);

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
    
        number_holder <uint32_t> data_begin;
        std::string_view data_end;
    };

    typedef ship_requests::ship_armament_lt_t::searchers ship_searchers_t;
    typedef ship_requests::armament_info_t::searchers searcher_t;
    
    std::vector <response_t, allocator_for_temp <response_t> >
    response (int id, std::chrono::year_month_day date) const;
    
    p_response_t partial_response (searcher_t const & searcher);

    struct ship_items_lt
    {
        ship_items_lt (size_t _searcher_id, ship_searchers_t const & value) :
            searcher_id(_searcher_id),
            count    (value.count),
            date_from(value.date_from),
            date_to  (value.date_to)
        {}
    
        size_t searcher_id;
        uint32_t count;
        std::optional <std::chrono::year_month_day> date_from;
        std::optional <std::chrono::year_month_day> date_to;
    };
    
private:
    std::unordered_map <int, std::vector <ship_items_lt> > ship_searchers_list;
    std::vector <p_response_t> searchers;
    std::unordered_map <int, std::string> cache_class_names;
    
    std::string new_line;
};


#endif

