#include "ship_names.h"

#include "ship_info.h"
#include "ship_event.h"
#include "date_to_str.h"


ship_names::ship_names (header_column _table, ship_requests * _database) :
    table(_table),
    database(_database)
{
    std::vector <ship_t> ships =
        database->ship_info.get_list();
    for (ship_t & ship : ships)   
    {
        int ship_id = ship.ship_id;
        ship_list_cache.insert({ship_id, std::move(ship)});
    }
    
    std::vector <event_t> events =
        database->ship_event.get_event_lt();
    for (event_t & event : events)
    {
        int ship_id = event.ship_id;
        ship_events[ship_id].push_back(std::move(event));
    }
}

bool ship_names::on_modernization (int ship_id, std::chrono::year_month_day date)
{
    std::unordered_map <int, std::vector <event_t> > :: iterator it = ship_events.find(ship_id);
    if (it == ship_events.end())
        return 0;
    for (event_t const & event : it->second)
    {
        if ((!event.date_from || event.date_from <= date) &&
            (!event.date_to   || event.date_to > date)) [[unlikely]]
            return 1;
    }
    return 0;
}

ship_names::response_t ship_names::response (std::vector <std::pair <int, std::chrono::year_month_day> > ship_year)
{
    response_t answer = 
    {
        std::string(table.begin),
        std::vector <uint8_t> (ship_year.size())
    };

    for (size_t i = 0; i != ship_year.size(); ++i)
    {
        if (i != 0)
            answer.row += table.new_column;
        
        std::unordered_map <int, ship_t> :: iterator ship = ship_list_cache.find(ship_year[i].first);
        if (ship == ship_list_cache.end())
            continue;
        
        answer.row += ship->second.ship_ru.value_or("");
        if (ship->second.class_ru || ship->second.type_ru)
        {
            answer.row.append(table.new_line).append("(");
            if (ship->second.class_ru)
                answer.row.append(*ship->second.class_ru).append(" "); 
            if (ship->second.type_ru)
                answer.row.append("типа ").append(*(ship->second.type_ru));
            answer.row.append(")");
        }
        answer.row.append(table.new_line).append(to_string(ship_year[i].second));

        bool modernizations = on_modernization(ship_year[i].first, ship_year[i].second);

        answer.modernization[i] = modernizations;
        if (modernizations)
            answer.row.append(table.new_line).append("на модернизации");
        if (ship->second.commissioned && ship_year[i].second < *ship->second.commissioned)
            answer.row.append(table.new_line).append("еще не введен в строй");
        if (ship->second.sunk_date && ship_year[i].second > *ship->second.sunk_date)
            answer.row.append(table.new_line).append("потоплен");
    }

    answer.row += table.end;
    return answer;
}

