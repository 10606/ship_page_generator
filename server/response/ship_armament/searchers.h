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
    ship_searchers (ship_requests * _database, std::string_view _new_line);

    struct response_t
    {
        int group;
        bool compare;
        std::string group_name;
        std::string data;
    };

    std::vector <response_t> response (int id, std::chrono::year_month_day date);

private:
    typedef ship_requests::ship_armament_lt_t::searchers ship_searchers_t;
    typedef ship_requests::armament_info_t::searchers searcher_t;
    
    std::unordered_map <int, std::vector <ship_searchers_t> > ship_searchers_list;
    std::unordered_map <int, response_t> searchers;
    
    response_t partial_response (searcher_t const & searcher);
    
    ship_requests * database;
    std::string new_line;
};


#endif

