#ifndef RESPONSE_SHIP_ARMAMENT_H
#define RESPONSE_SHIP_ARMAMENT_H

#include <compare>
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
#include "ship_names.h"
#include "simple_string.h"

#include "general.h"
#include "guns.h"
#include "torpedo.h"
#include "throwers.h"
#include "searchers.h"
#include "catapult.h"
#include "aircraft.h"
#include "propulsion.h"


struct rows_table_template
{
    rows_table_template (std::string_view tr_class = std::string_view())
    {
        if (!tr_class.empty())
            one_delimeter = group_delimeter = std::string("</tr>\n<tr ").append(tr_class).append(">\n");
    }

    struct column_t
    {
        std::string_view begin = "<td>";
        std::string_view new_column = "</td>\n<td>";
        std::string_view new_line = "<br>\n";
        std::string_view end = "</td>\n";
    };
    column_t column;
    
    std::string one_delimeter = "</tr>\n<tr>\n";
    std::string group_delimeter = "</tr>\n<tr>\n";

    struct rowspan_t
    {
        // <begin> number <middle> text <end>
        std::string_view begin = "<th rowspan=";
        std::string_view middle = ">"; 
        std::string_view end = "</th>\n";
    };
    rowspan_t rowspan;
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
    
    template <typename T, typename V>
    struct key_impl
    {
        key_impl (T const & _group, V const & _compare) :
            group(_group),
            compare(_compare)
        {}
        
        T group;
        V compare;
        
        std::partial_ordering operator <=> (key_impl const &) const = default; 
    };
    
    template <typename T, typename V>
    requires (std::is_const_v <T> && !std::is_const_v <V>)
    struct key_impl <T, V>
    {
        key_impl (T const &, V const & _compare) :
            compare(_compare)
        {}
        
        static const constexpr bool group = 0;
        V compare;
        
        std::strong_ordering operator <=> (key_impl const &) const = default; 
    };
    
    template <typename T, typename V>
    requires (!std::is_const_v <T> && std::is_const_v <V>)
    struct key_impl <T, V>
    {
        key_impl (T const & _group, V const &) :
            group(_group)
        {}
        
        T group;
        static const constexpr bool compare = 0;
        
        std::strong_ordering operator <=> (key_impl const &) const = default; 
    };
    
    template <typename T, typename V>
    requires (std::is_const_v <T> && std::is_const_v <V>)
    struct key_impl <T, V>
    {
        key_impl (T const &, V const &)
        {}
        
        static const constexpr bool group = 0;
        static const constexpr bool compare = 0;
        
        std::strong_ordering operator <=> (key_impl const &) const = default; 
    };
    
    using response_t = typename responser::response_t;
    using key_t = key_impl <decltype(std::declval <response_t>().group),
                            decltype(std::declval <response_t>().compare)>;

    void response
    (
        simple_string & answer,
        std::vector <std::pair <int, std::chrono::year_month_day> > const & ship_year,
        std::vector <uint8_t> const & modernization
    ) const;
    
private:
    rows_table_template table;
    responser resp;
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
    ship_armament (ship_requests * _database) :
        names(header_column(), _database),
        general      (rows_table_template("class=\"general\""     ), _database, table.new_line),
        guns         (rows_table_template("class=\"guns\""        ), _database, table.new_line),
        torpedo_tubes(rows_table_template("class=\"torpedo\""     ), _database, table.new_line),
        throwers     (rows_table_template("class=\"throwers\""    ), _database, table.new_line),
        searchers    (rows_table_template("class=\"searchers\""   ), _database, table.new_line),
        catapult     (rows_table_template("class=\"catapult\""    ), _database, table.new_line),
        aircraft     (rows_table_template("class=\"aircraft\""    ), _database, table.new_line),
        propulsion   (rows_table_template("class=\"propulsion\""  ), _database, table.new_line),
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
    void response (simple_string & answer, std::string_view query, piece_t title, bool add_checkbox = 0);
    void response (simple_string & answer, std::vector <std::pair <int, std::chrono::year_month_day> > const & ship_year, bool add_checkbox = 0);

private:
    struct table_template
    {
        std::string_view begin = "<table border=1 class=\"ship_armament\">\n<tr class=\"header\">\n";
        std::string_view new_line = "<br>\n";
        std::string_view end = "</tr>\n</table>\n";
    };
    table_template table;

    ship_names names;
    ships_responser <ship_general>       general;
    ships_responser <ship_guns>          guns;
    ships_responser <ship_torpedo_tubes> torpedo_tubes;
    ships_responser <ship_throwers>      throwers;
    ships_responser <ship_searchers>     searchers;
    ships_responser <ship_catapult>      catapult;
    ships_responser <ship_aircrafts>     aircraft;
    ships_responser <ship_propulsion>    propulsion;

    std::vector <std::pair <int, std::chrono::year_month_day> > parse_query__ship_year (std::string_view query);
    std::unordered_map <int, std::chrono::year_month_day> default_date;
    
    static const constexpr std::string_view style = "";
};


#endif

