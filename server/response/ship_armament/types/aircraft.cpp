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
    std::vector <ship_aircrafts_t> aircraft_list =
        database->ship_armament_lt.get_aircraft("");

    for (ship_aircrafts_t & aircraft : aircraft_list)
        ship_aircrafts_list[aircraft.ship_id].push_back(std::move(aircraft));

    // sorting
    {
        std::vector <aircraft_t> mounts_list =
            database->aircraft_info.get_list("");
        std::unordered_map <int, aircraft_t> aircrafts_full;
        for (aircraft_t & aircraft : mounts_list)
        {
            int aircraft_id = aircraft.id;
            aircrafts.insert({aircraft_id, partial_response(aircraft)});
            aircrafts_full.insert({aircraft_id, std::move(aircraft)});
        }
        
        auto torpedo_order = 
            [&aircrafts_full] (ship_aircrafts_t const & a, ship_aircrafts_t const & b) -> bool
            {
                // class_id, aircraft_id
                aircraft_t const & a_info = aircrafts_full.at(a.aircraft_id);
                aircraft_t const & b_info = aircrafts_full.at(b.aircraft_id);
                
                std::strong_ordering class_cmp = a_info.class_id <=> b_info.class_id;
                if (class_cmp != std::strong_ordering::equal)
                    return std::is_lt(class_cmp);
                    
                return a.aircraft_id < b.aircraft_id;
            };
        
        for (auto & item : ship_aircrafts_list)
            std::sort(item.second.begin(), item.second.end(), torpedo_order);
    }
}

std::vector <ship_aircrafts::response_t> ship_aircrafts::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_aircrafts_t> > :: const_iterator it = ship_aircrafts_list.find(id);
    if (it == ship_aircrafts_list.end())
        return answer;
    for (ship_aircrafts_t const & aircraft : it->second)
    {
        if (between(aircraft.date_from, date, aircraft.date_to))
        {
            std::unordered_map <int, response_t> :: const_iterator air_it = aircrafts.find(aircraft.aircraft_id);
            response_t item = (air_it != aircrafts.end())? air_it->second : response_t();
            item.data = std::to_string(aircraft.count) + " " + item.data;
            answer.push_back(item);
            answer.back().group_name = group_name;
        }
    }
    
    return answer;
}

ship_aircrafts::response_t ship_aircrafts::partial_response (aircraft_t const & aircraft)
{
    response_t item;
    item.compare = aircraft.class_id;
    item.group = 0;
    
    if (aircraft.aircraft_en)
        item.data += *aircraft.aircraft_en;
    if (aircraft.aircraft_en && aircraft.aircraft_ru) 
        item.data += new_line;
    if (aircraft.aircraft_ru)
        item.data += "&emsp;" + 
                    armament_links::filtered
                    (
                        "/aircraft?group=type&sort=in_service", 
                        aircraft.aircraft_ru.value_or("  "), 
                        aircraft.class_id
                    );

    return item;
}


