#ifndef SHIP_ARMAMENT_H
#define SHIP_ARMAMENT_H

#include "ship_requests.h"

    
struct ship_requests::ship_armament_t::guns
{
    guns (pqxx::row const & value);
    
    int class_id;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    int gun_id;
    std::optional <double> caliber;
    std::optional <double> length;
    std::optional <std::string> gun_ru;
    std::optional <std::string> gun_en;
    std::optional <std::string> mount_ru;
    std::optional <std::string> mount_en;
    std::optional <double> angle;
    uint32_t gun_count;
    uint32_t mount_count;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};


struct ship_requests::ship_armament_t::torpedo_tubes
{
    torpedo_tubes (pqxx::row const & value);
    
    int tube_id;
    std::optional <double> caliber;
    uint32_t tubes_count;
    std::optional <std::string> tube_ru;
    std::optional <std::string> tube_en;
    uint32_t mount_count;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};


struct ship_requests::ship_armament_t::throwers
{
    throwers (pqxx::row const & value);
    
    int class_id;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    int throwers_id;
    std::optional <double> caliber;
    uint32_t tubes_count;
    std::optional <std::string> thrower_ru;
    std::optional <std::string> thrower_en;
    uint32_t mount_count;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};


struct ship_requests::ship_armament_t::searchers
{
    searchers (pqxx::row const & value);
    
    int class_id;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    int searcher_id;
    std::optional <std::string> searcher_ru;
    std::optional <std::string> searcher_en;
    uint32_t count;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};


struct ship_requests::ship_armament_t::catapult
{
    catapult (pqxx::row const & value);
    
    int class_id;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    int catapult_id;
    std::optional <std::string> catapult_ru;
    std::optional <std::string> catapult_en;
    uint32_t count;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};
    

struct ship_requests::ship_armament_t::aircraft
{
    aircraft (pqxx::row const & value);
    
    int class_id;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    int aircraft_id;
    std::optional <std::string> aircraft_ru;
    std::optional <std::string> aircraft_en;
    uint32_t count;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};
    

#endif

