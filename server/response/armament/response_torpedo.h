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
    
    std::string response (std::string_view query);
    
private:
    std::vector <std::vector <torpedo_t> >
    torpedo_group_and_sort
    (std::string_view query);

    ship_requests * database;
};


#endif

