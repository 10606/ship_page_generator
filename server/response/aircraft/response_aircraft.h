#ifndef RESPONSE_aircraft_H
#define RESPONSE_aircraft_H

#include "aircraft_info.h"
#include "response_partial.h"
#include "simple_string.h"
#include "parse_query.h"


struct aircraft
{
    aircraft (ship_requests * database) :
        aircraft_cache(),
        text_cache(),
        pictures_cache(),
        guns(),
        armament()
    {
        std::vector <aircraft_t> tmp = database->aircraft_info.get_list("");
        aircraft_cache = partial::partial_response <aircraft_t, aircraft_partial> (tmp);
        text_cache = partial::text_response <aircraft_t, aircraft_text> (tmp);

        std::vector <picture_t> pictures_list = database->pictures.get_aircraft();
        pictures_cache = partial::pictures_response <aircraft_t> (pictures_list, tmp);

        fill_aircraft_armament(database, tmp);
    }
    
    typedef ship_requests::pictures_t::picture picture_t;
    typedef ship_requests::aircraft_info_t::list aircraft_t;
 
    // https://127.0.0.1:8443/aircraft?sort=in_service&group=type&filter=in_service,3x,4x&filter=class,0
    void response (simple_string & answer, std::string_view query, piece_t title);
    
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
    
        double mass;             /* kg */
        double engine_power;     /* hp */
        double max_speed;        /* km/h */
        double range;            /* km */
        double range_with_tank;  /* km */
        double ceiling;          /* km */
        double time_to_altitude; /* 1000m in ... minutes */
        std::optional <std::chrono::year_month_day> in_service;
    };
    
    struct link_to_gun
    {
        link_to_gun (size_t _gun_index, uint32_t _count) :
            gun_index(_gun_index),
            count(_count)
        {}
        
        size_t gun_index;
        uint32_t count;
    };

    void fill_aircraft_armament
    (
        ship_requests * database,
        std::vector <aircraft_t> const & aircraft_list
    );
    
private:
    std::vector <aircraft_partial> aircraft_cache;
    std::vector <aircraft_text> text_cache;
    std::vector <std::vector <picture_t> > pictures_cache;

    std::vector <std::string> guns;
    std::vector <std::vector <link_to_gun> > armament;
};


#endif

