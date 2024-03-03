#ifndef RESPONSE_TORPEDO_H
#define RESPONSE_TORPEDO_H

#include "armament_info.h"
#include "response_partial.h"
#include "simple_string.h"
#include "parse_query.h"


struct torpedo
{
    torpedo (ship_requests * database) :
        torpedo_cache(),
        text_cache()
    {
        std::vector <torpedo_t> tmp = database->armament_info.get_torpedo();
        torpedo_cache = partial::partial_response <torpedo_t, torpedo_partial> (tmp);
        text_cache = partial::text_response <torpedo_t, torpedo_text> (tmp);
        for (size_t i = 0; i != torpedo_cache.size(); ++i)
        {
            torpedo_cache[i].name_ru = text_cache[i].name_ru;
            torpedo_cache[i].name_en = text_cache[i].name_en;
        }
    }
    
    typedef ship_requests::armament_info_t::torpedo torpedo_t;
 
    // https://127.0.0.1:8443/armament/torpedo?sort=in_service,mass_ex&group=caliber&filter=in_service,3x,4x&filter=caliber,450,533,610
    void response (simple_string & answer, std::string_view query, piece_t title);
    
    struct torpedo_text
    {
        torpedo_text (torpedo_t const & value);
            
        std::string name;
        std::optional <std::string> name_ru;
        std::optional <std::string> name_en;
        std::string caliber;
        std::string length;
        std::string speed;
        std::string range;
        std::string mass;
        std::string mass_ex;
        std::string in_service;
    };
    
    struct torpedo_partial
    {
        torpedo_partial (torpedo_t const & value, size_t _index);
    
        size_t index;
        
        int id;
        std::optional <std::string_view> name_ru;
        std::optional <std::string_view> name_en;
    
        double caliber;
        double mass_ex;
        std::optional <std::chrono::year_month_day> in_service;
    };
    
private:
    std::vector <torpedo_partial> torpedo_cache;
    std::vector <torpedo_text> text_cache;
};


#endif

