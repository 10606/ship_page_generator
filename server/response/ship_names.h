#ifndef SHIP_NAMES_H
#define SHIP_NAMES_H

#include <string_view>
#include <unordered_map>
#include <vector>
#include "html_template.h"
#include "ship_info.h"
#include "ship_event.h"
#include "ship_requests.h"
#include "simple_string.h"


struct header_column
{
    std::string_view begin = "<th></th>\n<th>\n";
    std::string_view new_column = "</th>\n<th>\n";
    std::string_view new_line = "<br>\n";
    std::string_view end = "</th>\n";
};

struct ship_names
{
    ship_names (header_column _table, ship_requests * database);

    std::vector <uint8_t> /* on modernization? */
    response
    (
        simple_string & answer, 
        std::vector <std::pair <int, std::chrono::year_month_day> > ship_year, 
        bool add_checkbox = 0
    );

private:
    typedef ship_requests::ship_info_t::list ship_t;
    struct cache_info
    {
        std::string answer;
        std::optional <std::chrono::year_month_day> commissioned;
        std::optional <std::chrono::year_month_day> sunk_date;
    };
    std::unordered_map <int, cache_info> ship_list_cache;

    typedef ship_requests::ship_event_t::event_lt event_t;
    std::unordered_map <int, std::vector <event_t> > ship_events;
    
    bool on_modernization (int ship_id, std::chrono::year_month_day date);
    
    std::string ship_info (ship_t const & ship);

    header_column table;
    static const constexpr html_template_3 checkbox =
    {
        "<input type=\"checkbox\" onchange=\"toggle_to_detail_compare(event)\" ship=", 
        " date=\"",
        "\"></input>"
    };
};

#endif

