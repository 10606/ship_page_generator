#ifndef RESPONSE_TORPEDO_TUBES_H
#define RESPONSE_TORPEDO_TUBES_H

#include "armament_info.h"
#include "response_partial.h"


struct torpedo_tubes
{
    torpedo_tubes (ship_requests * database) :
        torpedo_tubes_cache(),
        text_cache()
    {
        std::vector <torpedo_tubes_t> tmp = database->armament_info.get_torpedo_tubes();
        torpedo_tubes_cache = partial::partial_response <torpedo_tubes_t, torpedo_tubes_partial> (tmp);
        text_cache = partial::text_response <torpedo_tubes_t, torpedo_tubes_text> (tmp);
    }
    
    typedef ship_requests::armament_info_t::torpedo_tubes torpedo_tubes_t;
 
    // https://127.0.0.1:8443/armament/torpedo_tubes?sort=in_service&group=caliber&filter=in_service,3x,4x&filter=caliber,450,533,610
    void response (std::string & answer, std::string_view query);
    
    struct torpedo_tubes_text
    {
        torpedo_tubes_text (torpedo_tubes_t const & value);
            
        std::string name;
        std::string caliber;
        std::string tubes_count;
        std::string in_service;
    };
    
    struct torpedo_tubes_partial
    {
        torpedo_tubes_partial (torpedo_tubes_t const & value, size_t _index);
    
        size_t index;
        
        int id;
        int class_id;
        std::optional <std::string> name_ru;
        std::optional <std::string> name_en;
    
        double caliber;
        std::optional <std::chrono::year_month_day> in_service;
    };
    
private:
    std::vector <torpedo_tubes_partial> torpedo_tubes_cache;
    std::vector <torpedo_tubes_text> text_cache;
};


#endif

