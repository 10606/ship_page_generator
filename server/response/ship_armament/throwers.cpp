#include "throwers.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"


ship_throwers::ship_throwers (ship_requests * _database, std::string_view _new_line) :
    database(_database),
    new_line(_new_line)
{
    std::vector <ship_throwers_t> tube_list =
        database->ship_armament_lt.get_throwers("");

    for (ship_throwers_t & tube : tube_list)
        ship_throwers_list[tube.ship_id].push_back(std::move(tube));

    // sorting
    {
        std::vector <throwers_t> mounts_list =
            database->armament_info.get_throwers();
        std::unordered_map <int, throwers_t> throwers_full;
        for (throwers_t & thrower : mounts_list)
        {
            int thrower_id = thrower.id;
            throwers.insert({thrower_id, partial_response(thrower)});
            throwers_full.insert({thrower_id, std::move(thrower)});
        }
        
        auto torpedo_order = 
            [&throwers_full] (ship_throwers_t const & a, ship_throwers_t const & b) -> bool
            {
                // class_id, -caliber, tube_count, thrower_id
                throwers_t const & a_info = throwers_full.at(a.throwers_id);
                throwers_t const & b_info = throwers_full.at(b.throwers_id);
                
                std::strong_ordering class_cmp = a_info.class_id <=> b_info.class_id;
                if (class_cmp != std::strong_ordering::equal)
                    return std::is_lt(class_cmp);
                    
                if ((a_info.caliber && !b_info.caliber) ||
                    (!a_info.caliber && b_info.caliber))
                    return a_info.caliber.has_value(); // null last
                if ((a_info.caliber && b_info.caliber) &&
                    (*a_info.caliber != *b_info.caliber))
                    return *b_info.caliber < *a_info.caliber;
                    
                std::strong_ordering cnt_cmp = a_info.tubes_count <=> b_info.tubes_count;
                if (cnt_cmp != std::strong_ordering::equal)
                    return std::is_lt(cnt_cmp);
                    
                return a.throwers_id < b.throwers_id;
            };
        
        for (auto & item : ship_throwers_list)
            std::sort(item.second.begin(), item.second.end(), torpedo_order);
    }
}

std::vector <ship_throwers::response_t> ship_throwers::response (int id, std::chrono::year_month_day date)
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_throwers_t> > :: iterator it = ship_throwers_list.find(id);
    if (it == ship_throwers_list.end())
        return answer;
    for (ship_throwers_t const & thrower : it->second)
    {
        if (between(thrower.date_from, date, thrower.date_to))
        {
            response_t item = throwers[thrower.throwers_id];
            item.data = std::to_string(thrower.mount_count) + item.data;
            answer.push_back(item);
        }
    }
    
    return answer;
}

ship_throwers::response_t ship_throwers::partial_response (throwers_t const & thrower)
{
    response_t item;
    item.group = 0;
    item.group_name = "противолодочное вооружение";
    item.compare = 0;
    
    item.data += "x" + (thrower.tubes_count? std::to_string(*thrower.tubes_count) : "") + " ";
    item.data += (thrower.caliber? (to_string_10(*thrower.caliber) + "мм  ") : "  ");
    item.data += (thrower.class_ru? *thrower.class_ru + "  ": "");
    item.data += thrower.thrower_ru.value_or("  ");
    return item;
}


