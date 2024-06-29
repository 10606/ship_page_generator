#ifndef SHIP_ARMAMENT_GUNS_H
#define SHIP_ARMAMENT_GUNS_H

#include <vector>
#include <unordered_map>
#include <map>
#include <chrono>
#include <cmath>
#include "allocator.h"
#include "armament_info.h"
#include "ship_armament_lt.h"
#include "get_segments.h"


struct ship_guns
{
    ship_guns (ship_requests * database, std::string_view _new_line);

    struct p_response_t
    {
        int group;
        double compare;
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
        double compare;
        std::string_view group_name;
        char data_begin[16];
        std::string_view data_end;
    };

    typedef ship_requests::ship_armament_lt_t::guns ship_guns_t;
    typedef ship_requests::armament_info_t::mount mount_t;
    
    std::vector <response_t, allocator_for_temp <response_t> >
    response (int id, std::chrono::year_month_day date) const;

    p_response_t partial_response (mount_t const & mount);

    struct ship_items_lt
    {
        ship_items_lt (size_t _mount_id, ship_guns_t const & value) :
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
    
private:
    struct mount_info_t
    {
        size_t mount_id;
        uint32_t mount_count;
    };

    typedef get_segments
            <
                std::optional <std::chrono::year_month_day>,
                mount_info_t
            > segments_t;

    typedef segments_t::data_t segment_data;

    std::unordered_map <int, segments_t> ship_guns_list_segmented;

    std::vector <p_response_t> mounts;
    std::unordered_map <int, std::string> cache_class_names;

    std::string new_line;
};


#endif

