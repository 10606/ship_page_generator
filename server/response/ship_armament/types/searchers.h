#ifndef SEARCHERS_H
#define SEARCHERS_H

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "armament_info.h"
#include "ship_armament_lt.h"


struct ship_searchers
{
    ship_searchers (ship_requests * database, std::string_view _new_line);

    struct p_response_t
    {
        int group;
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
    
        int group;
        bool compare;
        std::string_view group_name;
        char data_begin[16];
        std::string_view data_end;
    };

    typedef ship_requests::ship_armament_lt_t::searchers ship_searchers_t;
    typedef ship_requests::armament_info_t::searchers searcher_t;
    
    std::vector <response_t> response (int id, std::chrono::year_month_day date) const;
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

