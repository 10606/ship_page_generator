#ifndef RESPONSE_SEARCHER_H
#define RESPONSE_SEARCHER_H

#include "armament_info.h"
#include "response_partial.h"
#include "simple_string.h"
#include "parse_query.h"


struct searcher
{
    searcher (ship_requests * database) :
        searchers_cache(),
        text_cache()
    {
        std::vector <searcher_t> tmp = database->armament_info.get_searchers();
        searchers_cache = partial::partial_response <searcher_t, searchers_partial> (tmp);
        text_cache = partial::text_response <searcher_t, searchers_text> (tmp);

        std::vector <picture_t> pictures_list = database->pictures.get_searcher();
        pictures_cache = partial::pictures_response <searcher_t> (pictures_list, tmp);
    }
    
    typedef ship_requests::pictures_t::picture picture_t;
    typedef ship_requests::armament_info_t::searchers searcher_t;
 
    // https://127.0.0.1:8443/armament/searcher?sort=power,in_service&group=class&filter=in_service,3x,4x
    void response (simple_string & answer, std::string_view query, piece_t title);
    
    struct searchers_text
    {
        searchers_text (searcher_t const & value);
        std::string freq_convert (double frequency);
            
        std::string name;
        std::string mass;
        std::string frequency;
        std::string power;
        std::string build_cnt;
        std::string in_service;
    };
    
    struct searchers_partial
    {
        searchers_partial (searcher_t const & value, size_t _index);
    
        size_t index;
        
        int id;
        int class_id;
        std::optional <std::string> name_ru;
        std::optional <std::string> name_en;
    
        double mass;
        double power;
        int power_group;
        std::optional <std::chrono::year_month_day> in_service;
    };
    
private:
    std::vector <searchers_partial> searchers_cache;
    std::vector <searchers_text> text_cache;
    std::vector <std::vector <picture_t> > pictures_cache;
};


#endif

