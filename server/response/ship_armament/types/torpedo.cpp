#include "torpedo.h"

#include "ship_requests.h"
#include "ship_armament.h"
#include "ship_armament_lt.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "armament_links.h"


ship_torpedo_tubes::ship_torpedo_tubes (ship_requests * _database, std::string_view _new_line) :
    database(_database),
    new_line(_new_line),
    group_name(armament_links::base("/armament/torpedo?group=caliber&sort=in_service", "торпедный аппарат"))
{
    std::vector <ship_tubes_t> tube_list =
        database->ship_armament_lt.get_torpedo_tubes("");

    for (ship_tubes_t & tube : tube_list)
        ship_tubes_list[tube.ship_id].push_back(std::move(tube));

    // sorting
    {
        std::vector <tube_t> mounts_list =
            database->armament_info.get_torpedo_tubes();
        std::unordered_map <int, tube_t> torpedo_tubes_full;
        for (tube_t & tube : mounts_list)
        {
            int tube_id = tube.id;
            torpedo_tubes.insert({tube_id, partial_response(tube)});
            torpedo_tubes_full.insert({tube_id, std::move(tube)});
        }
        
        auto torpedo_order = 
            [&torpedo_tubes_full] (ship_tubes_t const & a, ship_tubes_t const & b) -> bool
            {
                // class_id, -caliber, tube_count, tube_id
                tube_t const & a_info = torpedo_tubes_full.at(a.tube_id);
                tube_t const & b_info = torpedo_tubes_full.at(b.tube_id);
                
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
                    
                return a.tube_id < b.tube_id;
            };
        
        for (auto & item : ship_tubes_list)
            std::sort(item.second.begin(), item.second.end(), torpedo_order);
    }
}

std::vector <ship_torpedo_tubes::response_t> ship_torpedo_tubes::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_tubes_t> > :: const_iterator it = ship_tubes_list.find(id);
    if (it == ship_tubes_list.end())
        return answer;
    for (ship_tubes_t const & tube : it->second)
    {
        if (between(tube.date_from, date, tube.date_to))
        {
            std::unordered_map <int, p_response_t> :: const_iterator torpedo_it = torpedo_tubes.find(tube.tube_id);
            response_t item = (torpedo_it != torpedo_tubes.end())? torpedo_it->second : response_t();
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


