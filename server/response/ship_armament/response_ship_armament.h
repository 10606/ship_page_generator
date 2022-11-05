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
#include "simple_string.h"


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
        resp(_database, std::forward <T> (args) ...)
    {}
    
    using response_t = typename responser::response_t;
    using key_t = std::pair <decltype(std::declval <response_t>().group),
                             decltype(std::declval <response_t>().compare)>;

    void response
    (
        simple_string & answer,
        std::vector <std::pair <int, std::chrono::year_month_day> > const & ship_year,
        std::vector <uint8_t> const & modernization
    ) const;
    
    std::vector <std::pair <uint32_t, std::string_view> >
    gun_classes
    (
        std::vector <std::vector <typename responser::response_t> > & values,
        std::optional <key_t> min
    ) const;

private:
    rows_table_template table;
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
            begin = std::string("<table border=1>\n")
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
void add_armament 
(
    simple_string & answer,
    ships_responser <armament_type> const & armament, 
    std::vector <std::pair <int, std::chrono::year_month_day> > const & ship_year,
    std::vector <uint8_t> const & modernizations
)
{
    try
    {
        armament.response(answer, ship_year, modernizations);
    }
    catch (...)
    {}
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
        aircraft     (rows_table_template("class = \"aircraft\"" ), _database, table.new_line),
        default_date()
    {
        std::vector <ship_requests::ship_info_t::list> ships_info = _database->ship_info.get_list();
        for (ship_requests::ship_info_t::list const & value : ships_info)
            if (value.commissioned)
                default_date.insert({value.ship_id, *value.commissioned});
    }

    bool check (std::string_view uri)
    {
        return uri == "/ship/armament";
    }

    // http://127.0.0.1:8080/ship/armament?ship=40&date=9.7.44&ship=42&date=14.7.44&ship=43&date=8.7.44&ship=50&date=30.1.39&date=9.7.44&ship=52&date=9.7.44&ship=54&date=23.7.45
    void response (simple_string & answer, std::string_view query, piece_t title);

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

    std::vector <std::pair <int, std::chrono::year_month_day> > parse_query__ship_year (std::string_view query);
    std::unordered_map <int, std::chrono::year_month_day> default_date;
    
    static const constexpr std::string_view style = "";
};


#endif

