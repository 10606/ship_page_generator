#ifndef RESPONSE_CATAPULT_H
#define RESPONSE_CATAPULT_H

#include "armament_info.h"
#include "response_partial.h"


struct catapult
{
    catapult (ship_requests * database) :
        catapult_cache(),
        text_cache()
    {
        std::vector <catapult_t> tmp = database->armament_info.get_catapult();
        catapult_cache = partial::partial_response <catapult_t, catapult_partial> (tmp);
        text_cache = partial::text_response <catapult_t, catapult_text> (tmp);
    }
    
    typedef ship_requests::armament_info_t::catapult catapult_t;
 
    // https://127.0.0.1:8443/armament/catapult?sort=in_service,launch_mass&group=class
    void response (std::string & answer, std::string_view query);
    
    struct catapult_text
    {
        catapult_text (catapult_t const & value);
            
        std::string name;
        std::string length;
        std::string width;
        std::string speed;
        std::string launch_mass;
        std::string alleceration;
        std::string in_service;
    };
    
    struct catapult_partial
    {
        catapult_partial (catapult_t const & value, size_t _index);
    
        size_t index;
        
        int id;
        int class_id;
        std::optional <std::string> name_ru;
        std::optional <std::string> name_en;
    
        double acceleration;
        double speed;
        double launch_mass;
        std::optional <std::chrono::year_month_day> in_service;
    };
    
private:
    std::vector <catapult_partial> catapult_cache;
    std::vector <catapult_text> text_cache;
};


#endif

