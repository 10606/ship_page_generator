#ifndef THROWERS_H
#define THROWERS_H

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "armament_info.h"
#include "ship_armament_lt.h"


struct ship_throwers
{
    ship_throwers (ship_requests * _database, std::string_view _new_line);

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
        std::string data_begin;
        std::string_view data_end;
    };

    std::vector <response_t> response (int id, std::chrono::year_month_day date) const;

private:
    typedef ship_requests::ship_armament_lt_t::throwers ship_throwers_t;
    typedef ship_requests::armament_info_t::throwers throwers_t;
    
    std::unordered_map <int, std::vector <ship_throwers_t> > ship_throwers_list;
    std::unordered_map <int, p_response_t> throwers;
    
    p_response_t partial_response (throwers_t const & thrower);
    
    ship_requests * database;
    std::string new_line;
    
    std::string group_name;
};


#endif

