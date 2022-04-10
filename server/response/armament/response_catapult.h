#ifndef RESPONSE_CATAPULT_H
#define RESPONSE_CATAPULT_H

#include "armament_info.h"


struct catapult
{
    catapult (ship_requests * _database) :
       database(_database)
    {}
    
    typedef ship_requests::armament_info_t::catapult catapult_t;
 
    // https://127.0.0.1:8443/armament/catapult?sort=in_service,mass_ex&group=class
    std::string response (std::string_view query);
    
private:
    ship_requests * database;
};


#endif

