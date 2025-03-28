#ifndef RESPONSE_GUNS_H
#define RESPONSE_GUNS_H

#include "response.h"
#include "armament_info.h"
#include "response_partial.h"
#include "simple_string.h"
#include "parse_query.h"


struct guns : response_base
{
    guns (ship_requests & database) :
        guns_cache(),
        text_cache(),
        pictures_cache()
    {
        std::vector <guns_t> tmp = database.armament_info.get_list();
        guns_cache = partial::partial_response <guns_t, guns_partial> (tmp);
        text_cache = partial::text_response <guns_t, guns_text> (tmp);
        for (size_t i = 0; i != guns_cache.size(); ++i)
        {
            guns_cache[i].name_ru = text_cache[i].name_ru;
            guns_cache[i].name_en = text_cache[i].name_en;
        }

        std::vector <picture_t> pictures_list = database.pictures.get_gun();
        pictures_cache = partial::pictures_response <guns_t> (pictures_list, tmp);
    }
    
    typedef ship_requests::pictures_t::picture picture_t;
    typedef ship_requests::armament_info_t::list guns_t;
 
    // https://127.0.0.1:8443/armament/guns?sort=caliber,in_service&group=class&filter=in_service,2x,3x,4x
    virtual void response (simple_string &, std::string_view query, piece_t title) override;
    
    struct guns_text
    {
        guns_text (guns_t const & value);
            
        std::string name;
        std::optional <std::string> name_ru;
        std::optional <std::string> name_en;
        std::string caliber;
        std::string length;
        std::string rate_of_fire;
        std::string effective_range;
        std::string mass;
        std::string build_cnt;
        std::string in_service;
    };
    
    struct guns_partial
    {
        guns_partial (guns_t const & value, size_t _index);
    
        size_t index;
        
        int id;
        int class_id;
        std::optional <std::string_view> name_ru;
        std::optional <std::string_view> name_en;
    
        double caliber;
        int caliber_group;
        std::optional <std::chrono::year_month_day> in_service;
    };
    
private:
    std::vector <guns_partial> guns_cache;
    std::vector <guns_text> text_cache;
    std::vector <std::vector <picture_t> > pictures_cache;
};


#endif

