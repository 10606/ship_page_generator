#ifndef RESPONSE_aircraft_H
#define RESPONSE_aircraft_H

#include "aircraft_info.h"


struct aircraft
{
    aircraft (ship_requests * _database) :
       database(_database)
    {}
    
    typedef ship_requests::aircraft_info_t::list aircraft_t;
 
    // 
    std::string response (std::string_view query);
    
private:
    ship_requests * database;
};


#endif

