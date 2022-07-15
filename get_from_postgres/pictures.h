#ifndef PICTURES_H
#define PICTURES_H

#include "ship_requests.h"


struct ship_requests::pictures_t::picture
{
    picture (pqxx::row const & value);

    int id;
    std::string path_small;
    std::string path_full;
    std::string description;
};


#endif

