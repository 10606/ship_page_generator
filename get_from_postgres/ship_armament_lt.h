#ifndef SHIP_ARMAMENT_LT_H
#define SHIP_ARMAMENT_LT_H

#include "ship_requests.h"

    
struct ship_requests::ship_armament_lt_t::guns
{
    guns (pqxx::row const & value);
    
    int ship_id;
    int mount_id;
    uint32_t mount_count;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};


struct ship_requests::ship_armament_lt_t::torpedo_tubes
{
    torpedo_tubes (pqxx::row const & value);
    
    int ship_id;
    int tube_id;
    uint32_t mount_count;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};


struct ship_requests::ship_armament_lt_t::throwers
{
    throwers (pqxx::row const & value);
    
    int ship_id;
    int throwers_id;
    uint32_t mount_count;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};


struct ship_requests::ship_armament_lt_t::searchers
{
    searchers (pqxx::row const & value);
    
    int ship_id;
    int searcher_id;
    uint32_t count;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};


struct ship_requests::ship_armament_lt_t::catapult
{
    catapult (pqxx::row const & value);
    
    int ship_id;
    int catapult_id;
    uint32_t count;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};
    

struct ship_requests::ship_armament_lt_t::aircraft
{
    aircraft (pqxx::row const & value);
    
    int ship_id;
    int aircraft_id;
    uint32_t count;
    uint32_t count_reserve;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};
    

#endif

