#include "aircraft.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "aircraft_info.h"
#include "armament_links.h"


ship_aircrafts::ship_aircrafts (ship_requests * _database, std::string_view _new_line) :
    database(_database),
    new_line(_new_line),
    group_name(armament_links::base("/aircraft?group=class&sort=in_service", "авиагруппа"))
{
    std::vector <aircraft_t> aircrafts_full =
        database->aircraft_info.get_list("");
    std::unordered_map <int, size_t> aircrafts_index;
    for (aircraft_t & aircraft : aircrafts_full)
    {
        int aircraft_id = aircraft.id;
        aircrafts_index.insert({aircraft_id, aircrafts.size()});
        aircrafts.push_back(partial_response(aircraft));
    }
    
    std::vector <ship_aircrafts_t> aircraft_list =
        database->ship_armament_lt.get_aircraft("");
    for (ship_aircrafts_t & aircraft : aircraft_list)
    {
        std::unordered_map <int, size_t> ::iterator it = aircrafts_index.find(aircraft.aircraft_id);
        if (it != aircrafts_index.end())
            ship_aircrafts_list[aircraft.ship_id].emplace_back(it->second, aircraft);
    }

    // sorting
    {
        auto torpedo_order = 
            [&aircrafts_full] (ship_aircrafts_lt const & a, ship_aircrafts_lt const & b) -> bool
            {
                // class_id, aircraft_id
                aircraft_t const & a_info = aircrafts_full[a.aircraft_id];
                aircraft_t const & b_info = aircrafts_full[b.aircraft_id];
                
                std::strong_ordering class_cmp = a_info.class_id <=> b_info.class_id;
                if (class_cmp != std::strong_ordering::equal)
                    return std::is_lt(class_cmp);
                    
                return a_info.id < b_info.id;
            };
        
        for (auto & item : ship_aircrafts_list)
            std::sort(item.second.begin(), item.second.end(), torpedo_order);
    }
}

std::vector <ship_aircrafts::response_t> ship_aircrafts::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_aircrafts_lt> > :: const_iterator it = ship_aircrafts_list.find(id);
    if (it == ship_aircrafts_list.end())
        return answer;
    for (ship_aircrafts_lt const & aircraft : it->second)
    {
        if (between(aircraft.date_from, date, aircraft.date_to))
        {
            response_t item = aircrafts[aircraft.aircraft_id];
            add_value(item.data_begin, aircraft.count);
            answer.push_back(item);
            answer.back().group_name = group_name;
        }
    }
    
    return answer;
}

ship_aircrafts::p_response_t ship_aircrafts::partial_response (aircraft_t const & aircraft)
{
    p_response_t item;
    item.compare = aircraft.class_id;
    item.group = 0;
    
    item.data += " ";
    if (aircraft.aircraft_en)
        item.data += *aircraft.aircraft_en;
    if (aircraft.aircraft_en && aircraft.aircraft_ru) 
        item.data += new_line;
    if (aircraft.aircraft_ru)
        item.data += "&emsp;" + 
                    armament_links::filtered
                    (
                        "/aircraft?group=in_service&sort=type", 
                        aircraft.aircraft_ru.value_or("  "), 
                        aircraft.class_id
                    );

    return item;
}


