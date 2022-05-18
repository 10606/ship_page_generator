#ifndef RESPONSE_aircraft_H
#define RESPONSE_aircraft_H

#include "aircraft_info.h"


struct aircraft
{
    aircraft (ship_requests * _database) :
        aircraft_cache(),
        text_cache(),
        database(_database)
    {
        std::vector <aircraft_t> tmp = database->aircraft_info.get_list("");
        aircraft_cache = partial_response(tmp);
        text_cache = text_response(tmp);
    }
    
    typedef ship_requests::aircraft_info_t::list aircraft_t;
 
    // https://127.0.0.1:8443/aircraft?sort=in_service&group=type&filter=in_service,3x,4x&filter=class,0
    void response (std::string & answer, std::string_view query);
    
    struct aircraft_text
    {
        aircraft_text (aircraft_t const & value);
            
        std::string name;
        std::string crew;
        std::string mass;
        std::string max_mass;
        std::string engine_power;
        std::string max_speed;
        std::string cruise_speed;
        std::string range;
        std::string range_with_tank;
        std::string ceiling;
        std::string time_to_altitude;
        std::string in_service;
    };
    
    struct aircraft_partial
    {
        aircraft_partial (aircraft_t const & value, size_t _index);
    
        size_t index;
        
        int id;
        int type_id;
        int class_id;
        std::optional <std::string> aircraft_ru;
        std::optional <std::string> aircraft_en;
    
        std::optional <double> mass;             /* kg */
        std::optional <double> engine_power;     /* hp */
        std::optional <double> max_speed;        /* km/h */
        std::optional <double> range;            /* km */
        std::optional <double> range_with_tank;  /* km */
        std::optional <double> ceiling;          /* km */
        std::optional <double> time_to_altitude; /* 1000m in ... minutes */
        std::optional <std::chrono::year_month_day> in_service;
    };
    
private:
    static std::vector <aircraft_partial> partial_response (std::vector <aircraft_t> const & aircrafts);
    static std::vector <aircraft_text> text_response (std::vector <aircraft_t> const & aircrafts);
    
    std::vector <aircraft_partial> aircraft_cache;
    std::vector <aircraft_text> text_cache;
    
    ship_requests * database;
};


#endif

