#ifndef PICTURES_H
#define PICTURES_H

#include "ship_requests.h"


struct ship_requests::pictures_t::ship
{
    ship (pqxx::row const & value);

    int ship_id;
    std::string path_small;
    std::string path_full;
    std::string description;
};


struct ship_requests::pictures_t::aircraft
{
    aircraft (pqxx::row const & value);

    int aircraft_id;
    std::string path_small;
    std::string path_full;
    std::string description;
};


#endif

