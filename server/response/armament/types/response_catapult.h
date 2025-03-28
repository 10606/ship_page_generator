#ifndef RESPONSE_CATAPULT_H
#define RESPONSE_CATAPULT_H

#include "response.h"
#include "armament_info.h"
#include "response_partial.h"
#include "simple_string.h"
#include "parse_query.h"


struct catapult : response_base
{
    catapult (ship_requests & database) :
        catapult_cache(),
        text_cache()
    {
        std::vector <catapult_t> tmp = database.armament_info.get_catapult();
        catapult_cache = partial::partial_response <catapult_t, catapult_partial> (tmp);
        text_cache = partial::text_response <catapult_t, catapult_text> (tmp);
        for (size_t i = 0; i != catapult_cache.size(); ++i)
        {
            catapult_cache[i].name_ru = text_cache[i].name_ru;
            catapult_cache[i].name_en = text_cache[i].name_en;
        }

        std::vector <picture_t> pictures_list = database.pictures.get_catapult();
        pictures_cache = partial::pictures_response <catapult_t> (pictures_list, tmp);
    }
    
    typedef ship_requests::pictures_t::picture picture_t;
    typedef ship_requests::armament_info_t::catapult catapult_t;
 
    // https://127.0.0.1:8443/armament/catapult?sort=in_service,launch_mass&group=class
    virtual void response (simple_string & answer, std::string_view query, piece_t title) override;
    
    struct catapult_text
    {
        catapult_text (catapult_t const & value);
            
        std::string name;
        std::optional <std::string> name_ru;
        std::optional <std::string> name_en;
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
        std::optional <std::string_view> name_ru;
        std::optional <std::string_view> name_en;
    
        double acceleration;
        double speed;
        double launch_mass;
        std::optional <std::chrono::year_month_day> in_service;
    };
    
private:
    std::vector <catapult_partial> catapult_cache;
    std::vector <catapult_text> text_cache;
    std::vector <std::vector <picture_t> > pictures_cache;
};


#endif

