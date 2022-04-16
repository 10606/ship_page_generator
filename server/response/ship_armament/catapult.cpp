#include "catapult.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"


ship_catapult::ship_catapult (ship_requests * _database, std::string_view _new_line) :
    database(_database),
    new_line(_new_line)
{
    std::vector <ship_catapults_t> catapult_list =
        database->ship_armament_lt.get_catapult("");

    for (ship_catapults_t & catapult : catapult_list)
        ship_catapults_list[catapult.ship_id].push_back(std::move(catapult));

    // sorting
    {
        std::vector <catapult_t> mounts_list =
            database->armament_info.get_catapult();
        std::unordered_map <int, catapult_t> catapults_full;
        for (catapult_t & catapult : mounts_list)
        {
            int catapult_id = catapult.id;
            catapults.insert({catapult_id, partial_response(catapult)});
            catapults_full.insert({catapult_id, std::move(catapult)});
        }
        
        auto torpedo_order = 
            [&catapults_full] (ship_catapults_t const & a, ship_catapults_t const & b) -> bool
            {
                // class_id, catapult_id
                catapult_t const & a_info = catapults_full.at(a.catapult_id);
                catapult_t const & b_info = catapults_full.at(b.catapult_id);
                
                std::strong_ordering class_cmp = a_info.class_id <=> b_info.class_id;
                if (class_cmp != std::strong_ordering::equal)
                    return std::is_lt(class_cmp);
                    
                return a.catapult_id < b.catapult_id;
            };
        
        for (auto & item : ship_catapults_list)
            std::sort(item.second.begin(), item.second.end(), torpedo_order);
    }
}

std::vector <ship_catapult::response_t> ship_catapult::response (int id, std::chrono::year_month_day date)
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_catapults_t> > :: iterator it = ship_catapults_list.find(id);
    if (it == ship_catapults_list.end())
        return answer;
    for (ship_catapults_t const & catapult : it->second)
    {
        if (between(catapult.date_from, date, catapult.date_to))
        {
            response_t item = catapults[catapult.catapult_id];
            item.data = std::to_string(catapult.count) + " " + item.data;
            answer.push_back(item);
            answer.back().group_name = "катапульта";
        }
    }
    
    return answer;
}

ship_catapult::response_t ship_catapult::partial_response (catapult_t const & catapult)
{
    response_t item;
    item.group = 0;
    item.compare = catapult.class_id;;
    
    item.data += catapult.catapult_ru.value_or("  ");
    return item;
}
    
    
