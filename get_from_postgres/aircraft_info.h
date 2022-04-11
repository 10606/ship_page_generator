#ifndef AIRCRAFT_INFO_H
#define AIRCRAFT_INFO_H

#include "ship_requests.h"


struct ship_requests::aircraft_info_t::bombs
{
    bombs (pqxx::row const & value);
    
    int bomb_id;
    std::optional <std::string> bomb_ru;
    std::optional <std::string> bomb_en;
    
    std::optional <double> mass;    /* кг */
    std::optional <double> mass_ex; /* кг */
    std::optional <std::chrono::year_month_day> in_service;
};


struct ship_requests::aircraft_info_t::classes
{
    classes (pqxx::row const & value);
    
    int class_id;
    std::optional <int> parent_id;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
};
    
    
struct ship_requests::aircraft_info_t::types
{
    types (pqxx::row const & value);
    
    int type_id;
    std::optional <std::string> type_ru;
    std::optional <std::string> type_en;
};
    
    
struct ship_requests::aircraft_info_t::list
{
    list (pqxx::row const & value);
    
    int id;
    int type_id;
    int class_id;
    std::optional <std::string> aircraft_ru;
    std::optional <std::string> aircraft_en;
    std::optional <std::string> type_ru;
    std::optional <std::string> type_en;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;

    std::optional <uint32_t> crew;
    std::optional <double> mass;             /* kg */
    std::optional <double> max_mass;         /* kg */
    std::optional <double> engine_power;     /* hp */
    std::optional <double> max_speed;        /* km/h */
    std::optional <double> cruise_speed;     /* km/h */
    std::optional <double> range;            /* km */
    std::optional <double> range_with_tank;  /* km */
    std::optional <double> ceiling;          /* km */
    std::optional <double> time_to_altitude; /* 1000m in ... minutes */
    
    std::optional <std::chrono::year_month_day> in_service;
};
    
    
#endif

