#ifndef RESPONSE_MINES_CHARGES_H
#define RESPONSE_MINES_CHARGES_H

#include "armament_info.h"


struct mines_charges
{
    mines_charges (ship_requests * _database) :
       database(_database)
    {}
    
    typedef ship_requests::armament_info_t::mines_charges mines_charges_t;
 
    // https://127.0.0.1:8443/armament/mines_charges?sort=in_service,mass_ex&group=class
    void response (std::string & answer, std::string_view query);
    
private:
    ship_requests * database;
};


#endif

