#include "catapult.h"

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


ship_catapult::ship_catapult (ship_requests * database, std::string_view _new_line) :
    new_line(_new_line),
    group_name(armament_links::base("/armament/catapult?group=class&sort=in_service", "катапульта"))
{
    fill_data_structures
    <
        ship_catapult,
        ship_catapults_t,
        catapult_t,
        &ship_catapult::catapults,
        &ship_catapults_t::catapult_id
    >
    (
        *this, 
        database->armament_info.get_catapult(),
        database->ship_armament_lt.get_catapult(""),
        &ship_catapults_list,
        
        [] (std::vector <catapult_t> const & catapults_full, std::vector <size_t> const & old_index)
        {
            return
            [&catapults_full, &old_index] (ship_items_lt const & a, ship_items_lt const & b) -> bool
            {
                // class_id, catapult_id
                catapult_t const & a_info = catapults_full[old_index[a.catapult_id]];
                catapult_t const & b_info = catapults_full[old_index[b.catapult_id]];
                
                std::strong_ordering class_cmp = a_info.class_id <=> b_info.class_id;
                if (class_cmp != std::strong_ordering::equal)
                    return std::is_lt(class_cmp);
                    
                return a_info.id < b_info.id;
            };
        }
    );
}

std::vector <ship_catapult::response_t> ship_catapult::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_items_lt> > :: const_iterator it = ship_catapults_list.find(id);
    if (it == ship_catapults_list.end())
        return answer;
    for (ship_items_lt const & catapult : it->second)
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
    
    
