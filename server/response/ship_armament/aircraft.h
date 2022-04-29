#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "armament_info.h"
#include "ship_armament_lt.h"


struct ship_aircrafts
{
    ship_aircrafts (ship_requests * _database, std::string_view _new_line);

    struct response_t
    {
        bool group;
        int compare;
        std::string group_name;
        std::string data;
    };

    std::vector <response_t> response (int id, std::chrono::year_month_day date) const;

private:
    typedef ship_requests::ship_armament_lt_t::aircraft ship_aircrafts_t;
    typedef ship_requests::aircraft_info_t::list aircraft_t;
    
    std::unordered_map <int, std::vector <ship_aircrafts_t> > ship_aircrafts_list;
    std::unordered_map <int, response_t> aircrafts;
    
    response_t partial_response (aircraft_t const & aircraft);
    
    ship_requests * database;
    std::string new_line;
};


#endif

