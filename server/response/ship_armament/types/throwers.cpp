#include "throwers.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "armament_links.h"


ship_throwers::ship_throwers (ship_requests * database, std::string_view _new_line) :
    new_line(_new_line),
    group_name(armament_links::base("/armament/mines_charges?filter=class,4&sort=mass_ex", "противолодочное вооружение"))
{
    std::vector <throwers_t> throwers_full =
        database->armament_info.get_throwers();
    std::unordered_map <int, size_t> throwers_index;
    for (throwers_t & thrower : throwers_full)
    {
        int thrower_id = thrower.id;
        throwers_index.insert({thrower_id, throwers.size()});
        throwers.push_back(partial_response(thrower));
    }
    
    std::vector <ship_throwers_t> thrower_list =
        database->ship_armament_lt.get_throwers("");
    for (ship_throwers_t & thrower : thrower_list)
    {
        std::unordered_map <int, size_t> ::iterator it = throwers_index.find(thrower.throwers_id);
        if (it != throwers_index.end())
            ship_throwers_list[thrower.ship_id].emplace_back(it->second, thrower);
    }

    // sorting
    {
        auto torpedo_order = 
            [&throwers_full] (ship_throwers_lt const & a, ship_throwers_lt const & b) -> bool
            {
                // class_id, -caliber, tube_count, thrower_id
                throwers_t const & a_info = throwers_full[a.thrower_id];
                throwers_t const & b_info = throwers_full[b.thrower_id];
                
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
                    
                return a_info.id < b_info.id;
            };
        
        for (auto & item : ship_throwers_list)
            std::sort(item.second.begin(), item.second.end(), torpedo_order);
    }
}

std::vector <ship_throwers::response_t> ship_throwers::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_throwers_lt> > :: const_iterator it = ship_throwers_list.find(id);
    if (it == ship_throwers_list.end())
        return answer;
    for (ship_throwers_lt const & thrower : it->second)
    {
        if (between(thrower.date_from, date, thrower.date_to))
        {
            response_t item = throwers[thrower.thrower_id];
            add_value(item.data_begin, thrower.mount_count);
            answer.push_back(item);
            answer.back().group_name = group_name;
        }
    }
    
    return answer;
}

ship_throwers::p_response_t ship_throwers::partial_response (throwers_t const & thrower)
{
    p_response_t item;
    item.group = 0;
    item.compare = 0;
    
    item.data += "x";
    if (thrower.tubes_count)
        add_value(item.data, *thrower.tubes_count);
    item.data += " ";
    item.data += (thrower.caliber? (to_string_10(*thrower.caliber) + "мм  ") : "  ");
    item.data += (thrower.class_ru? *thrower.class_ru + "  ": "");
    item.data += thrower.thrower_ru.value_or("  ");
    return item;
}


