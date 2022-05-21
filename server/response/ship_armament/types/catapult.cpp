#include "catapult.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "armament_links.h"


ship_catapult::ship_catapult (ship_requests * _database, std::string_view _new_line) :
    database(_database),
    new_line(_new_line),
    group_name(armament_links::base("/armament/catapult?group=class&sort=in_service", "катапульта"))
{
    std::vector <catapult_t> catapults_full =
        database->armament_info.get_catapult();
    std::unordered_map <int, size_t> catapults_index;
    for (catapult_t & catapult : catapults_full)
    {
        int catapult_id = catapult.id;
        catapults_index.insert({catapult_id, catapults.size()});
        catapults.push_back(partial_response(catapult));
    }
    
    std::vector <ship_catapults_t> catapult_list =
        database->ship_armament_lt.get_catapult("");
    for (ship_catapults_t & catapult : catapult_list)
    {
        std::unordered_map <int, size_t> ::iterator it = catapults_index.find(catapult.catapult_id);
        if (it != catapults_index.end())
            ship_catapults_list[catapult.ship_id].emplace_back(it->second, catapult);
    }

    // sorting
    {
        auto torpedo_order = 
            [&catapults_full] (ship_catapults_lt const & a, ship_catapults_lt const & b) -> bool
            {
                // class_id, catapult_id
                catapult_t const & a_info = catapults_full[a.catapult_id];
                catapult_t const & b_info = catapults_full[b.catapult_id];
                
                std::strong_ordering class_cmp = a_info.class_id <=> b_info.class_id;
                if (class_cmp != std::strong_ordering::equal)
                    return std::is_lt(class_cmp);
                    
                return a_info.id < b_info.id;
            };
        
        for (auto & item : ship_catapults_list)
            std::sort(item.second.begin(), item.second.end(), torpedo_order);
    }
}

std::vector <ship_catapult::response_t> ship_catapult::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_catapults_lt> > :: const_iterator it = ship_catapults_list.find(id);
    if (it == ship_catapults_list.end())
        return answer;
    for (ship_catapults_lt const & catapult : it->second)
    {
        if (between(catapult.date_from, date, catapult.date_to))
        {
            response_t item = catapults[catapult.catapult_id];
            add_value(item.data_begin, catapult.count);
            answer.push_back(item);
            answer.back().group_name = group_name;
        }
    }
    
    return answer;
}

ship_catapult::p_response_t ship_catapult::partial_response (catapult_t const & catapult)
{
    p_response_t item;
    item.group = 0;
    item.compare = catapult.class_id;;
    
    item.data += " ";
    item.data += catapult.catapult_ru.value_or("  ");
    if (catapult.class_ru)
        item.data.append("<br>&emsp;(")
                 .append(*catapult.class_ru)
                 .append(")");
    return item;
}
    
    
