#include "aircraft.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"

std::vector <ship_aircrafts::response_t> ship_aircrafts::response (int id, std::chrono::year_month_day date)
{
    std::vector <ship_requests::ship_armament_t::aircraft> aircrafts =
        database->ship_armament.get_aircraft(where("ship_aircraft", id, date) +
                                             "order by (aircraft_list.class_id, aircraft_list.id)");

    std::vector <response_t> answer;
    answer.reserve(aircrafts.size());

    for (auto aircraft : aircrafts)
    {
        response_t item;
        item.compare = aircraft.class_id;
        item.group = 0;
        item.group_name = "авиагруппа";
        
        item.data += std::to_string(aircraft.count) + " ";
        if (aircraft.aircraft_en)
            item.data += aircraft.aircraft_en.value_or("  ");
        if (aircraft.aircraft_en && aircraft.aircraft_ru) 
            item.data += new_line;
        if (aircraft.aircraft_ru)
            item.data += "&emsp;" + aircraft.aircraft_ru.value_or("  ");

        answer.push_back(item);
    }
    
    return answer;
}

