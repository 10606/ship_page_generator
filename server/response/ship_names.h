#ifndef SHIP_NAMES_H
#define SHIP_NAMES_H

#include <string_view>
#include <vector>
#include "ship_info.h"
#include "ship_event.h"
#include "ship_requests.h"


struct header_column
{
    std::string_view begin = "\n<th>\n</th>\n<th>\n";
    std::string_view new_column = "\n</th>\n<th>\n";
    std::string_view new_line = "<br>\n";
    std::string_view end = "\n</th>\n";
};

struct ship_names
{
    ship_names (header_column _table, ship_requests * database);

    struct response_t
    {
        std::string row;
        std::vector <uint8_t> modernization;
    };
    
    response_t response (std::vector <std::pair <int, std::chrono::year_month_day> > ship_year);

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
};

#endif

