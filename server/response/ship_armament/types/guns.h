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
    ship_guns (ship_requests * _database, std::string_view _new_line);

    struct p_response_t
    {
        int group;
        double compare;
        std::string group_name;
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
        double compare;
        std::string_view group_name;
        std::string data_begin;
        std::string_view data_end;
    };

    std::vector <response_t> response (int id, std::chrono::year_month_day date) const;

private:
    typedef ship_requests::ship_armament_lt_t::guns ship_guns_t;
    typedef ship_requests::armament_info_t::mount mount_t;
    
    struct ship_guns_lt
    {
        ship_guns_lt (size_t _mount_id, ship_guns_t const & value) :
            mount_id(_mount_id),
            mount_count(value.mount_count),
            date_from  (value.date_from),
            date_to    (value.date_to)
        {}
        
        size_t mount_id;
        uint32_t mount_count;
        std::optional <std::chrono::year_month_day> date_from;
        std::optional <std::chrono::year_month_day> date_to;
    };
    
    std::unordered_map <int, std::vector <ship_guns_lt> > ship_guns_list;
    std::vector <p_response_t> mounts;

    template <typename T>
    p_response_t partial_response (T const & mount);
    
    ship_requests * database;
    std::string new_line;
};


#endif

