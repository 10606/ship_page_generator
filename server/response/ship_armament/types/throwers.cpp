#include "throwers.h"

#include <vector>
#include <chrono>
#include <cmath>
#include <set>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "armament_links.h"
#include "common.h"


ship_throwers::ship_throwers (ship_requests & database, std::string_view _new_line) :
    new_line(_new_line),
    group_name(armament_links::base("/armament/mines_charges?filter=class,4&sort=mass_ex", "противолодочное вооружение"))
{
    fill_data_structures
    <
        ship_throwers,
        ship_throwers_t,
        throwers_t,
        &ship_throwers::throwers,
        &ship_throwers_t::throwers_id
    >
    (
        *this, 
        database.armament_info.get_throwers(),
        database.ship_armament_lt.get_throwers(""),
        &ship_throwers_list,
        
        [] (std::vector <throwers_t> const & throwers_full, std::vector <size_t> const & old_index)
        {
            return
            [&throwers_full, &old_index] (ship_items_lt const & a, ship_items_lt const & b) -> bool
            {
                // class_id, -caliber, tube_count, thrower_id
                throwers_t const & a_info = throwers_full[old_index[a.thrower_id]];
                throwers_t const & b_info = throwers_full[old_index[b.thrower_id]];
                
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
        }
    );
}

std::vector <ship_throwers::response_t, allocator_for_temp <ship_throwers::response_t> >
ship_throwers::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t, allocator_for_temp <response_t> > answer;

    std::unordered_map <int, std::vector <ship_items_lt> > :: const_iterator it = ship_throwers_list.find(id);
    if (it == ship_throwers_list.end())
        return answer;
    for (ship_items_lt const & thrower : it->second)
    {
        if (between(thrower.date_from, date, thrower.date_to))
        {
            answer.emplace_back(throwers[thrower.thrower_id]);
            add_value(answer.back().data_begin, thrower.mount_count);
            answer.back().group_name = group_name;
        }
    }
    
    return answer;
}

ship_throwers::p_response_t ship_throwers::partial_response (throwers_t const & thrower)
{
    p_response_t item;
    
    item.data += "x";
    if (thrower.tubes_count)
        add_value(item.data, *thrower.tubes_count);
    item.data += " ";
    item.data += (thrower.caliber? (to_string_10(*thrower.caliber) + "мм ") : " ");
    item.data += (thrower.class_ru? *thrower.class_ru + " ": "");
    if (thrower.thrower_ru)
        item.data.append("<b>")
                 .append(*thrower.thrower_ru)
                 .append("</b>");
    return item;
}


