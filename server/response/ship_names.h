#ifndef SHIP_NAMES_H
#define SHIP_NAMES_H

#include <string_view>
#include <vector>
#include "ship_requests.h"


struct header_column
{
    std::string_view begin = "\n<th>\n";
    std::string_view new_column = "\n</th>\n<th>\n";
    std::string_view new_line = "<br>\n";
    std::string_view end = "\n</th>\n";
};

struct ship_names
{
    ship_names (header_column _table, ship_requests * _database) :
        table(_table),
        database(_database)
    {}

    struct response_t
    {
        std::string row;
        std::vector <uint8_t> modernization;
    };
    
    response_t response (std::vector <std::pair <int, std::chrono::year_month_day> > ship_year);

private:
    header_column table;
    ship_requests * database;
};

#endif

