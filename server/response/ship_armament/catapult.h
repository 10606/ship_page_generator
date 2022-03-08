#ifndef CATAPULT_H
#define CATAPULT_H

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"


struct ship_catapult
{
    ship_catapult (ship_requests * _database, std::string_view _new_line) :
        database(_database),
        new_line(_new_line)
    {}

    struct response_t
    {
        bool group;
        int compare;
        std::string group_name;
        std::string data;
    };

    std::vector <response_t> response (int id, std::chrono::year_month_day date);

private:
    ship_requests * database;
    std::string new_line;
};


#endif

