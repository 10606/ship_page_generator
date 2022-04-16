#ifndef SHIP_ARMAMENT_GUNS_H
#define SHIP_ARMAMENT_GUNS_H

#include <vector>
#include <map>
#include <chrono>
#include <cmath>
#include "armament_info.h"
#include "ship_armament_lt.h"


struct ship_guns
{
    typedef ship_requests::armament_info_t::mount mount_t;
    typedef ship_requests::ship_armament_lt_t::guns ship_guns_t;
    
    ship_guns (ship_requests * _database, std::string_view _new_line);

    struct response_t
    {
        int group;
        double compare;
        std::string group_name;
        std::string data;
    };

    std::vector <response_t> response (int id, std::chrono::year_month_day date);

private:
    std::unordered_map <int, std::vector <ship_guns_t> > ship_guns_list;
    std::unordered_map <int, response_t> mounts;

    template <typename T>
    response_t partial_response (T const & mount);
    
    ship_requests * database;
    std::string new_line;
};


#endif

