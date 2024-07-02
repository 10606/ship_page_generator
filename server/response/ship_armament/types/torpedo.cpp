#include "torpedo.h"

#include <set>
#include "ship_requests.h"
#include "ship_armament.h"
#include "ship_armament_lt.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "armament_links.h"
#include "common.h"


ship_torpedo_tubes::ship_torpedo_tubes (ship_requests * database, std::string_view _new_line) :
    new_line(_new_line),
    group_name(armament_links::base("/armament/torpedo?group=caliber,in_service&sort=in_service", "торпедный аппарат"))
{
    fill_data_structures
    <
        ship_torpedo_tubes,
        ship_tubes_t,
        tube_t,
        &ship_torpedo_tubes::torpedo_tubes,
        &ship_tubes_t::tube_id
    >
    (
        *this, 
        database->armament_info.get_torpedo_tubes(),
        database->ship_armament_lt.get_torpedo_tubes(""),
        &ship_tubes_list,
        
        [] (std::vector <tube_t> const & torpedo_tubes_full, std::vector <size_t> const & old_index)
        {
            return
            [&torpedo_tubes_full, &old_index] (ship_items_lt const & a, ship_items_lt const & b) -> bool
            {
                // class_id, -caliber, tube_count, tube_id
                tube_t const & a_info = torpedo_tubes_full[old_index[a.tube_id]];
                tube_t const & b_info = torpedo_tubes_full[old_index[b.tube_id]];
                
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

std::vector <ship_torpedo_tubes::response_t, allocator_for_temp <ship_torpedo_tubes::response_t> >
ship_torpedo_tubes::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t, allocator_for_temp <response_t> > answer;

    std::unordered_map <int, std::vector <ship_items_lt> > :: const_iterator it = ship_tubes_list.find(id);
    if (it == ship_tubes_list.end())
        return answer;
    for (ship_items_lt const & tube : it->second)
    {
        if (between(tube.date_from, date, tube.date_to))
        {
            response_t item = torpedo_tubes[tube.tube_id];
            add_value(item.data_begin, tube.mount_count);
            answer.push_back(item);
            answer.back().group_name = group_name;
        }
    }
    
    return answer;
}

ship_torpedo_tubes::p_response_t ship_torpedo_tubes::partial_response (tube_t const & tube)
{
    p_response_t item;
    item.compare = tube.class_id;;
    
    item.data.append("x<b>");
    if (tube.tubes_count)
        add_value(item.data, *tube.tubes_count);
    item.data += " ";
    if (tube.caliber)
        item.data.append(to_string_10(*tube.caliber))
                 .append("мм");
    item.data.append(" ")
             .append(tube.tube_ru.value_or("  "));
    item.data.append("</b>")
             .append(new_line);
    item.data.append("&emsp;(")
             .append(tube.class_ru.value_or(""))
             .append(")")
             .append(new_line);
    return item;
}


