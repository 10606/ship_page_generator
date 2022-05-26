#ifndef RESPONSE_GUNS_H
#define RESPONSE_GUNS_H

#include "armament_info.h"
#include "response_partial.h"


struct guns
{
    guns (ship_requests * database) :
        guns_cache(),
        text_cache()
    {
        std::vector <guns_t> tmp = database->armament_info.get_list();
        guns_cache = partial::partial_response <guns_t, guns_partial> (tmp);
        text_cache = partial::text_response <guns_t, guns_text> (tmp);
    }
    
    typedef ship_requests::armament_info_t::list guns_t;
 
    // https://127.0.0.1:8443/armament/guns?sort=caliber,in_service&group=class&filter=in_service,2x,3x,4x
    void response (std::string & answer, std::string_view query);
    
    struct guns_text
    {
        guns_text (guns_t const & value);
            
        std::string name;
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
        std::optional <std::string> name_ru;
        std::optional <std::string> name_en;
    
        double caliber;
        std::optional <std::chrono::year_month_day> in_service;
    };
    
private:
    std::vector <guns_partial> guns_cache;
    std::vector <guns_text> text_cache;
};


#endif

