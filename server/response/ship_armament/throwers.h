#ifndef THROWERS_H
#define THROWERS_H

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"


struct ship_throwers
{
    ship_throwers (ship_requests * _database, std::string_view _new_line) :
        database(_database),
        new_line(_new_line)
    {}

    struct response_t
    {
        int group;
        bool compare;
        std::string group_name;
        std::string data;
    };

    std::vector <response_t> response (int id, std::chrono::year_month_day date);

private:
    ship_requests * database;
    std::string new_line;
};


#endif

