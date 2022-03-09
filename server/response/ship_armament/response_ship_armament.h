#ifndef RESPONSE_SHIP_ARMAMENT_H
#define RESPONSE_SHIP_ARMAMENT_H

#include <string>
#include <string_view>
#include <vector>
#include <chrono>
#include <optional>
#include <map>
#include <deque>
#include <iostream>
#include "parse_query.h"
#include "date_to_str.h"
#include "ship_requests.h"
#include "ship_armament.h"
#include "general.h"
#include "guns.h"
#include "torpedo.h"
#include "throwers.h"
#include "searchers.h"
#include "catapult.h"
#include "aircraft.h"
#include "ship_names.h"


struct rows_table_template
{
    struct column_t
    {
        std::string_view begin = "\n<td>\n";
        std::string_view new_column = "\n</td>\n<td>\n";
        std::string_view new_line = "<br>\n";
        std::string_view end = "\n</td>\n";
    } column = column_t();
    
    std::string_view one_delimeter = "\n</tr>\n<tr>\n";
    std::string_view group_delimeter = "\n</tr>\n<tr>\n";

    struct rowspan_t
    {
        // <begin> number <middle> text <end>
        std::string_view begin = "<th rowspan=";
        std::string_view middle = ">"; 
        std::string_view end = "</th>";
    } rowspan = rowspan_t();
};


template <typename responser>
struct ships_responser
{
    template <typename ... T>
    ships_responser 
    (
        rows_table_template _table,
        ship_requests * _database,
        T && ... args
    ) :
        table(_table),
        database(_database),
        resp(_database, std::forward <T> (args) ...)
    {}
    
    std::vector <std::string> response 
    (
        std::vector <std::pair <int, std::chrono::year_month_day> > ship_year,
        std::vector <uint8_t> modernization
    );
    

private:
    rows_table_template table;
    ship_requests * database;
    responser resp;
};


struct table_template
{
    std::string_view begin = "<table border=1>\n<tr>\n";
    std::string_view new_line = "<br>\n";
    std::string_view new_row = "\n</tr>\n<tr>\n";
    std::string_view end = "\n</tr>\n</table>\n";
};


template <typename armament_type>
std::string add_armament 
(
    ships_responser <armament_type> & armament, 
    std::vector <std::pair <int, std::chrono::year_month_day> > ship_year,
    std::vector <uint8_t> & modernizations,
    std::string_view new_row
)
{
    std::string answer(new_row);
    try
    {
        std::vector <std::string> table_guns = armament.response(ship_year, modernizations);
        for (auto const & s : table_guns)
            answer += s;
    }
    catch (...)
    {}
    
    return answer;
}


struct ship_armament
{
    ship_armament (table_template _table, ship_requests * _database) :
        table(_table),
        names(header_column(), _database),
        general(rows_table_template(), _database, table.new_line),
        guns(rows_table_template(), _database, table.new_line),
        torpedo_tubes(rows_table_template(), _database, table.new_line),
        throwers(rows_table_template(), _database, table.new_line),
        searchers(rows_table_template(), _database, table.new_line),
        catapult(rows_table_template(), _database, table.new_line),
        aircraft(rows_table_template(), _database, table.new_line)
    {}

    bool check (std::string_view uri)
    {
        return uri == "/ship/armament";
    }

    std::string response (std::string_view query);

private:
    table_template table;
    ship_names names;
    ships_responser <ship_general> general;
    ships_responser <ship_guns> guns;
    ships_responser <ship_torpedo_tubes> torpedo_tubes;
    ships_responser <ship_throwers> throwers;
    ships_responser <ship_searchers> searchers;
    ships_responser <ship_catapult> catapult;
    ships_responser <ship_aircrafts> aircraft;
};


#endif

