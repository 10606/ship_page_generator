#include "torpedo.h"

#include "ship_requests.h"
#include "ship_armament.h"
#include "ship_armament_lt.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "armament_links.h"


ship_torpedo_tubes::ship_torpedo_tubes (ship_requests * database, std::string_view _new_line) :
    new_line(_new_line),
    group_name(armament_links::base("/armament/torpedo?group=caliber&sort=in_service", "торпедный аппарат"))
{
    std::vector <tube_t> torpedo_tubes_full =
        database->armament_info.get_torpedo_tubes();
    std::unordered_map <int, size_t> torpedo_tubes_index;
    for (tube_t & tube : torpedo_tubes_full)
    {
        int tube_id = tube.id;
        torpedo_tubes_index.insert({tube_id, torpedo_tubes.size()});
        torpedo_tubes.push_back(partial_response(tube));
    }
    
    std::vector <ship_tubes_t> tube_list =
        database->ship_armament_lt.get_torpedo_tubes("");
    for (ship_tubes_t & tube : tube_list)
    {
        std::unordered_map <int, size_t> ::iterator it = torpedo_tubes_index.find(tube.tube_id);
        if (it != torpedo_tubes_index.end())
            ship_tubes_list[tube.ship_id].emplace_back(it->second, tube);
    }

    // sorting
    {
        auto torpedo_order = 
            [&torpedo_tubes_full] (ship_tubes_lt const & a, ship_tubes_lt const & b) -> bool
            {
                // class_id, -caliber, tube_count, tube_id
                tube_t const & a_info = torpedo_tubes_full[a.tube_id];
                tube_t const & b_info = torpedo_tubes_full[b.tube_id];
                
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
        
        for (auto & item : ship_tubes_list)
            std::sort(item.second.begin(), item.second.end(), torpedo_order);
    }
}

std::vector <ship_torpedo_tubes::response_t> ship_torpedo_tubes::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_tubes_lt> > :: const_iterator it = ship_tubes_list.find(id);
    if (it == ship_tubes_list.end())
        return answer;
    for (ship_tubes_lt const & tube : it->second)
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
    item.group = 0;
    item.compare = tube.class_id;;
    
    item.data += "x";
    if (tube.tubes_count)
        add_value(item.data, *tube.tubes_count);
    item.data += " ";
    item.data += (tube.caliber? (to_string_10(*tube.caliber) + "мм  ") : "  ");
    item.data += tube.tube_ru.value_or("  ") + new_line;
    return item;
}


