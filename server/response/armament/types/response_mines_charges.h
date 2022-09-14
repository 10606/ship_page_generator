#ifndef RESPONSE_MINES_CHARGES_H
#define RESPONSE_MINES_CHARGES_H

#include "armament_info.h"
#include "response_partial.h"
#include "simple_string.h"
#include "parse_query.h"


struct mines_charges
{
    mines_charges (ship_requests * database) :
        mines_charges_cache(),
        text_cache()
    {
        std::vector <mines_charges_t> tmp = database->armament_info.get_mines_charges();
        mines_charges_cache = partial::partial_response <mines_charges_t, mines_charges_partial> (tmp);
        text_cache = partial::text_response <mines_charges_t, mines_charges_text> (tmp);
    }
    
    typedef ship_requests::armament_info_t::mines_charges mines_charges_t;
 
    // https://127.0.0.1:8443/armament/mines_charges?sort=in_service,mass_ex&group=class
    void response (simple_string & answer, std::string_view query, piece_t title);
    
    struct mines_charges_text
    {
        mines_charges_text (mines_charges_t const & value);
            
        std::string name;
        std::string mass;
        std::string mass_ex;
        std::string size;
        std::string in_service;
    };
    
    struct mines_charges_partial
    {
        mines_charges_partial (mines_charges_t const & value, size_t _index);
    
        size_t index;
        
        int id;
        int class_id;
        std::optional <std::string> name_ru;
        std::optional <std::string> name_en;
    
        double mass_ex;
        std::optional <std::chrono::year_month_day> in_service;
    };
    
private:
    std::vector <mines_charges_partial> mines_charges_cache;
    std::vector <mines_charges_text> text_cache;
};


#endif

