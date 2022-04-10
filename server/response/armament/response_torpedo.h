#ifndef RESPONSE_TORPEDO_H
#define RESPONSE_TORPEDO_H

#include "armament_info.h"


struct torpedo
{
    torpedo (ship_requests * _database) :
       database(_database)
    {}
    
    typedef ship_requests::armament_info_t::torpedo torpedo_t;
 
    bool check (std::string_view uri)
    {
        return uri == "/armament/torpedo";
    }
    
    // https://127.0.0.1:8443/armament/torpedo?sort=in_service,mass_ex&group=caliber&filter=in_service,3x,4x&filter=caliber,450,533,610
    std::string response (std::string_view query);
    
private:
    ship_requests * database;
};


#endif

