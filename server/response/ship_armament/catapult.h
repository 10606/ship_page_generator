#ifndef CATAPULT_H
#define CATAPULT_H

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "armament_info.h"
#include "ship_armament_lt.h"


struct ship_catapult
{
    ship_catapult (ship_requests * _database, std::string_view _new_line);

    struct response_t
    {
        bool group;
        int compare;
        std::string_view group_name;
        std::string data;
    };

    std::vector <response_t> response (int id, std::chrono::year_month_day date) const;

private:
    typedef ship_requests::ship_armament_lt_t::catapult ship_catapults_t;
    typedef ship_requests::armament_info_t::catapult catapult_t;
    
    std::unordered_map <int, std::vector <ship_catapults_t> > ship_catapults_list;
    std::unordered_map <int, response_t> catapults;
    
    response_t partial_response (catapult_t const & catapult);
    
    ship_requests * database;
    std::string new_line;
};


#endif

