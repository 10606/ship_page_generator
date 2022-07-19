#include "aircraft.h"

#include <vector>
#include <chrono>
#include <cmath>
#include <set>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "aircraft_info.h"
#include "armament_links.h"


ship_aircrafts::ship_aircrafts (ship_requests * database, std::string_view _new_line) :
    new_line(_new_line)
{
    std::vector <aircraft_class> aircraft_class_list =
        database->aircraft_info.get_classes("");
    std::unordered_map <int, std::string> aircraft_class_map;
    
    for (aircraft_class const & cur_class : aircraft_class_list)
        aircraft_class_map.insert({cur_class.class_id, cur_class.class_ru.value_or(" ")});
    
    
    std::vector <aircraft_t> aircrafts_full =
        database->aircraft_info.get_list("");
    std::vector <ship_aircrafts_t> aircraft_list =
        database->ship_armament_lt.get_aircraft("");
    
    std::set <int> used; // add only used
    for (ship_aircrafts_t const & aircraft : aircraft_list)
        used.insert(aircraft.aircraft_id);

    std::unordered_map <int, size_t> aircrafts_index;
    aircrafts.reserve(used.size());
    for (aircraft_t & aircraft : aircrafts_full)
    {
        int aircraft_id = aircraft.id;
        if (used.find(aircraft_id) == used.end())
            continue;
        aircrafts_index.insert({aircraft_id, aircrafts.size()});
        aircrafts.push_back(partial_response(aircraft, aircraft_class_map));
    }
    
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
            item.data_begin = aircraft.count;
            answer.push_back(item);
        }
    }
    
    return answer;
}

ship_aircrafts::p_response_t ship_aircrafts::partial_response (aircraft_t const & aircraft, std::unordered_map <int, std::string> aircraft_class_map)
{
    p_response_t item;
    item.compare = 0;
    item.group = aircraft.class_id;
    
    std::unordered_map <int, std::string> :: iterator it = aircraft_class_map.find(aircraft.class_id);
    if (it != aircraft_class_map.end())
        item.group_name = armament_links::filtered
                          (
                              "/aircraft?group=in_service&sort=type,in_service", 
                              (it != aircraft_class_map.end())? it->second : " ",
                              aircraft.class_id
                          );
    
    item.data += " ";
    if (aircraft.aircraft_en)
        item.data += *aircraft.aircraft_en;
    if (aircraft.aircraft_en && aircraft.aircraft_ru) 
        item.data += new_line;
    if (aircraft.aircraft_ru)
        item.data += "&emsp;" + aircraft.aircraft_ru.value_or("  ");

    return item;
}


