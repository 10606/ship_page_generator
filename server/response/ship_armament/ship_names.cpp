#include "ship_names.h"

#include <iostream>
#include "ship_info.h"
#include "ship_event.h"
#include "date_to_str.h"


ship_names::ship_names (header_column _table, ship_requests & database) :
    table(_table)
{
    std::vector <ship_t> ships =
        database.ship_info.get_list();
    for (ship_t & ship : ships)   
    {
        int ship_id = ship.ship_id;
        ship_list_cache.insert({ship_id, cache_info{ship_info(ship), ship.commissioned, ship.sunk_date}});
    }
    
    std::vector <event_t> events =
        database.ship_event.get_event_lt("where class_id = 0 ");
    for (event_t & event : events)
    {
        int ship_id = event.ship_id;
        ship_events[ship_id].push_back(std::move(event));
    }
}

bool ship_names::on_modernization (int ship_id, std::chrono::year_month_day date)
{
    std::unordered_map <int, std::vector <event_t> > :: iterator it = ship_events.find(ship_id);
    if (it == ship_events.end()) [[unlikely]]
        return 0;
    for (event_t const & event : it->second)
    {
        if ((!event.date_from || event.date_from <= date) &&
            (!event.date_to   || event.date_to > date)) [[unlikely]]
            return 1;
    }
    return 0;
}

std::string ship_names::ship_info (ship_t const & ship)
{
    std::string answer;
    
    answer += ship.ship_ru.value_or("");
    if (ship.class_ru || ship.type_ru) [[likely]]
    {
        answer.append(table.new_line).append("(");
        if (ship.class_ru) [[likely]]
            answer.append(*ship.class_ru).append(" "); 
        if (ship.type_ru) [[likely]]
            answer.append("типа ").append(*(ship.type_ru));
        answer.append(")");
    }

    return answer;
}

std::vector <uint8_t> /* on modernization? */
ship_names::response (simple_string & answer, std::span <std::pair <int, std::chrono::year_month_day> const> ship_year, bool add_checkbox)
{
    std::vector <uint8_t> is_on_modernization (ship_year.size());
    answer.append(table.begin);

    for (size_t i = 0; i != ship_year.size(); ++i)
    {
        if (i != 0) [[likely]]
            answer.append(table.new_column);
        
        std::unordered_map <int, cache_info> :: iterator ship = ship_list_cache.find(ship_year[i].first);
        if (ship == ship_list_cache.end()) [[unlikely]]
            continue;
        
        bool modernizations = on_modernization(ship_year[i].first, ship_year[i].second);
        bool not_commissioned = ship->second.commissioned && ship_year[i].second < *ship->second.commissioned;
        bool sunk = ship->second.sunk_date && ship_year[i].second > *ship->second.sunk_date;
        
        answer.append(ship->second.answer, table.new_line);
        if (add_checkbox && !modernizations && !not_commissioned && !sunk)
        {
            answer.append
            (
                checkbox.begin,
                std::to_string(ship_year[i].first),
                checkbox.middle,
                to_string(ship_year[i].second),
                checkbox.end
            );
        }
        answer.append(to_string(ship_year[i].second));

        is_on_modernization[i] = modernizations;
        if (modernizations) [[unlikely]]
            answer.append(table.new_line).append("на модернизации");
        if (not_commissioned) [[unlikely]]
            answer.append(table.new_line).append("еще не введен в строй");
        if (sunk) [[unlikely]]
            answer.append(table.new_line).append("потоплен");
    }

    answer.append(table.end);
    return is_on_modernization;
}

