#ifndef RESPONSE_SEARCHER_H
#define RESPONSE_SEARCHER_H

#include "armament_info.h"


struct searcher
{
    searcher (ship_requests * _database) :
       database(_database)
    {}
    
    typedef ship_requests::armament_info_t::searchers searcher_t;
 
    // https://127.0.0.1:8443/armament/searcher?sort=power,in_service&group=class&filter=in_service,3x,4x
    std::string response (std::string_view query);
    
private:
    ship_requests * database;
};


#endif

