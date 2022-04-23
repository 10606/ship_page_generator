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
    rows_table_template (std::string_view tr_class = std::string_view())
    {
        if (!tr_class.empty())
            one_delimeter = group_delimeter = std::string("\n</tr>\n<tr ").append(tr_class).append(">\n");
    }

    struct column_t
    {
        std::string begin = "\n<td>\n";
        std::string new_column = "\n</td>\n<td>\n";
        std::string new_line = "<br>\n";
        std::string end = "\n</td>\n";
    } column = column_t();
    
    std::string one_delimeter = "\n</tr>\n<tr>\n";
    std::string group_delimeter = "\n</tr>\n<tr>\n";

    struct rowspan_t
    {
        // <begin> number <middle> text <end>
        std::string begin = "<th rowspan=";
        std::string middle = ">"; 
        std::string end = "</th>";
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
        std::vector <std::pair <int, std::chrono::year_month_day> > const & ship_year,
        std::vector <uint8_t> const & modernization
    );
    

private:
    rows_table_template table;
    ship_requests * database;
    responser resp;
};


struct table_template
{
    table_template
    (
        std::string_view style = std::string_view(),
        std::string_view tr_class = std::string_view()
    )
    {
        if (!style.empty() || !tr_class.empty())
            begin = std::string("<div class = \"main\">\n<table border=1>\n")
                    .append(style)
                    .append("<tr ")
                    .append(tr_class)
                    .append(">\n");
    }

    std::string begin = "<table border=1>\n<tr>\n";
    std::string new_line = "<br>\n";
    std::string end = "\n</tr>\n</table>\n";

    std::string new_row (std::string_view tr_class = std::string_view())
    {
        return std::string("\n</tr>\n<tr ").append(tr_class).append(">\n");
    };
};


template <typename armament_type>
std::string add_armament 
(
    ships_responser <armament_type> & armament, 
    std::vector <std::pair <int, std::chrono::year_month_day> > const & ship_year,
    std::vector <uint8_t> const & modernizations,
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
    ship_armament (ship_requests * _database, table_template _table = table_template(style, "class = \"header\"")) :
        table(_table),
        names(header_column(), _database),
        general      (rows_table_template("class = \"general\""  ), _database, table.new_line),
        guns         (rows_table_template("class = \"guns\""     ), _database, table.new_line),
        torpedo_tubes(rows_table_template("class = \"torpedo\""  ), _database, table.new_line),
        throwers     (rows_table_template("class = \"throwers\"" ), _database, table.new_line),
        searchers    (rows_table_template("class = \"searchers\""), _database, table.new_line),
        catapult     (rows_table_template("class = \"catapult\"" ), _database, table.new_line),
        aircraft     (rows_table_template("class = \"aircraft\"" ), _database, table.new_line)
    {}

    bool check (std::string_view uri)
    {
        return uri == "/ship/armament";
    }

    std::string response (std::string_view query);

private:
    table_template table;
    ship_names names;
    ships_responser <ship_general>       general;
    ships_responser <ship_guns>          guns;
    ships_responser <ship_torpedo_tubes> torpedo_tubes;
    ships_responser <ship_throwers>      throwers;
    ships_responser <ship_searchers>     searchers;
    ships_responser <ship_catapult>      catapult;
    ships_responser <ship_aircrafts>     aircraft;
    
    static const constexpr std::string_view style = 
        "<style type = \"text/css\"> \
            TR.header    { background: #fff8dc; } \
            TR.general   { background: #f8ffdc; } \
            TR.guns      { background: #f8ffff; } \
            TR.torpedo   { background: #fff8dc; } \
            TR.throwers  { background: #f8ffdc; } \
            TR.searchers { background: #f8ffff; } \
            TR.catapult  { background: #fff8dc; } \
            TR.aircraft  { background: #f8ffdc; } \
        </style>";
};


#endif

