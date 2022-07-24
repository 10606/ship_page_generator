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
#include "common.h"


ship_aircrafts::ship_aircrafts (ship_requests * database, std::string_view _new_line) :
    new_line(_new_line)
{
    std::vector <aircraft_class> aircraft_class_list =
        database->aircraft_info.get_classes("");
    std::unordered_map <int, std::string> aircraft_class_map;
    for (aircraft_class const & cur_class : aircraft_class_list)
        aircraft_class_map.insert({cur_class.class_id, cur_class.class_ru.value_or(" ")});
    
    fill_data_structures
    <
        ship_aircrafts,
        ship_aircrafts_t,
        ship_aircrafts_lt,
        aircraft_t,
        &ship_aircrafts::aircrafts,
        &ship_aircrafts::ship_aircrafts_list,
        &ship_aircrafts_t::aircraft_id
    >
    (
        *this, 
        database->aircraft_info.get_list(""),
        database->ship_armament_lt.get_aircraft(""),
        
        [] (std::vector <aircraft_t> const & aircrafts_full, std::vector <size_t> const & old_index)
        {
            return
            [&aircrafts_full, &old_index] (ship_aircrafts_lt const & a, ship_aircrafts_lt const & b) -> bool
            {
                // class_id, aircraft_id
                aircraft_t const & a_info = aircrafts_full[old_index[a.aircraft_id]];
                aircraft_t const & b_info = aircrafts_full[old_index[b.aircraft_id]];
                
                std::strong_ordering class_cmp = a_info.class_id <=> b_info.class_id;
                if (class_cmp != std::strong_ordering::equal)
                    return std::is_lt(class_cmp);
                    
                return a_info.id < b_info.id;
            };
        },
        aircraft_class_map
    );
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


