#ifndef GENERAL_H
#define GENERAL_H

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"


struct ship_general
{
    ship_general (ship_requests * _database, std::string_view _new_line) :
        database(_database),
        new_line(_new_line)
    {}

    struct response_t
    {
        bool group;
        uint8_t compare;
        std::string group_name;
        std::string data;
    };

    std::vector <response_t> response (int id, std::chrono::year_month_day date);

private:
    ship_requests * database;
    std::string new_line;
};


#endif

