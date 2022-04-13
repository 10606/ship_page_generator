#ifndef RESPONSE_aircraft_H
#define RESPONSE_aircraft_H

#include "aircraft_info.h"


struct aircraft
{
    aircraft (ship_requests * _database) :
       database(_database)
    {}
    
    typedef ship_requests::aircraft_info_t::list aircraft_t;
 
    // https://127.0.0.1:8443/aircraft?sort=in_service&group=type&filter=in_service,3x,4x&filter=class,0
    std::string response (std::string_view query);
    
private:
    ship_requests * database;
};


#endif

