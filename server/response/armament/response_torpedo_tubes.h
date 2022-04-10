#ifndef RESPONSE_TORPEDO_TUBES_H
#define RESPONSE_TORPEDO_TUBES_H

#include "armament_info.h"


struct torpedo_tubes
{
    torpedo_tubes (ship_requests * _database) :
       database(_database)
    {}
    
    typedef ship_requests::armament_info_t::torpedo_tubes torpedo_tubes_t;
 
    // https://127.0.0.1:8443/armament/torpedo_tubes?sort=in_service&group=caliber&filter=in_service,3x,4x&filter=caliber,450,533,610
    std::string response (std::string_view query);
    
private:
    ship_requests * database;
};


#endif

