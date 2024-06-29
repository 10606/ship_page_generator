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
    for (aircraft_class const & cur_class : aircraft_class_list)
        cache_class_names.insert
        (
            {
                cur_class.class_id,
                armament_links::filtered("/aircraft?group=in_service&sort=type,in_service", cur_class.class_ru.value_or(" "), cur_class.class_id)
            }
        );
    
    fill_data_structures
    <
        ship_aircrafts,
        ship_aircrafts_t,
        aircraft_t,
        &ship_aircrafts::aircrafts,
        &ship_aircrafts_t::aircraft_id
    >
    (
        *this, 
        database->aircraft_info.get_list(""),
        database->ship_armament_lt.get_aircraft(""),
        &ship_aircrafts_list,
        
        [] (std::vector <aircraft_t> const & aircrafts_full, std::vector <size_t> const & old_index)
        {
            return
            [&aircrafts_full, &old_index] (ship_items_lt const & a, ship_items_lt const & b) -> bool
            {
                // class_id, aircraft_id
                aircraft_t const & a_info = aircrafts_full[old_index[a.aircraft_id]];
                aircraft_t const & b_info = aircrafts_full[old_index[b.aircraft_id]];
                
                std::strong_ordering class_cmp = a_info.class_id <=> b_info.class_id;
                if (class_cmp != std::strong_ordering::equal)
                    return std::is_lt(class_cmp);
                    
                return a_info.id < b_info.id;
            };
        }
    );
}

std::vector <ship_aircrafts::response_t, allocator_for_temp <ship_aircrafts::response_t> >
ship_aircrafts::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t, allocator_for_temp <response_t> > answer;

    std::unordered_map <int, std::vector <ship_items_lt> > :: const_iterator it = ship_aircrafts_list.find(id);
    if (it == ship_aircrafts_list.end())
        return answer;
    for (ship_items_lt const & aircraft : it->second)
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

ship_aircrafts::p_response_t ship_aircrafts::partial_response (aircraft_t const & aircraft)
{
    p_response_t item;
    item.compare = 0;
    item.group = aircraft.class_id;
    
    std::unordered_map <int, std::string> :: iterator it = cache_class_names.find(aircraft.class_id);
    if (it != cache_class_names.end())
        item.group_name = it->second;
    item.data += " ";
    if (aircraft.aircraft_en)
        item.data.append("<b>")
                 .append(*aircraft.aircraft_en)
                 .append("</b>");
    if (aircraft.aircraft_en && aircraft.aircraft_ru) 
        item.data += new_line;
    if (aircraft.aircraft_ru)
        item.data += "&emsp;" + aircraft.aircraft_ru.value_or("  ");

    return item;
}


