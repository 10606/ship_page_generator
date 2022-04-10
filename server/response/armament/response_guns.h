#ifndef RESPONSE_GUNS_H
#define RESPONSE_GUNS_H

#include "armament_info.h"


struct guns
{
    guns (ship_requests * _database) :
       database(_database)
    {}
    
    typedef ship_requests::armament_info_t::list guns_t;
 
    // https://127.0.0.1:8443/
    std::string response (std::string_view query);
    
private:
    ship_requests * database;
};


#endif

