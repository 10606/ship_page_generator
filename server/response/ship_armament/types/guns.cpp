#include "guns.h"

#include <vector>
#include <chrono>
#include <cmath>
#include <set>
#include "ship_requests.h"
#include "ship_armament.h"
#include "ship_armament_lt.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "armament_links.h"
#include "common.h"


ship_guns::ship_guns (ship_requests * database, std::string_view _new_line) :
    ship_guns_list(),
    new_line(_new_line)
{
    fill_data_structures
    <
        ship_guns,
        ship_guns_t,
        mount_t,
        &ship_guns::mounts,
        &ship_guns::ship_guns_list,
        &ship_guns_t::mount_id
    >
    (
        *this, 
        database->armament_info.get_mount(),
        database->ship_armament_lt.get_guns(""),
        
        [] (std::vector <mount_t> const & mounts_full, std::vector <size_t> const & old_index)
        {
            return
            [&mounts_full, &old_index] (ship_items_lt const & a, ship_items_lt const & b) -> bool
            {
                // class_id, -caliber, gun_id, -gun_count, mount_id
                mount_t const & a_info = mounts_full[old_index[a.mount_id]];
                mount_t const & b_info = mounts_full[old_index[b.mount_id]];
                
                std::strong_ordering class_cmp = a_info.class_id <=> b_info.class_id;
                if (class_cmp != std::strong_ordering::equal)
                    return std::is_lt(class_cmp);
                    
                if ((a_info.caliber && !b_info.caliber) ||
                    (!a_info.caliber && b_info.caliber))
                    return a_info.caliber.has_value(); // null last
                if ((a_info.caliber && b_info.caliber) &&
                    (*a_info.caliber != *b_info.caliber))
                    return *b_info.caliber < *a_info.caliber;
                    
                std::strong_ordering gun_cmp = a_info.gun_id <=> b_info.gun_id;
                if (gun_cmp != std::strong_ordering::equal)
                    return std::is_lt(gun_cmp);
                
                std::strong_ordering cnt_cmp = a_info.gun_count <=> b_info.gun_count;
                if (cnt_cmp != std::strong_ordering::equal)
                    return std::is_gt(cnt_cmp);
                    
                return a_info.id < b_info.id;
            };
        }
    );
}


std::vector <ship_guns::response_t> ship_guns::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_items_lt> > :: const_iterator it = ship_guns_list.find(id);
    if (it == ship_guns_list.end())
        return answer;
    for (ship_items_lt const & value : it->second)
    {
        if (between(value.date_from, date, value.date_to))
        {
            response_t item = mounts[value.mount_id];
            add_value(item.data_begin, value.mount_count);
            answer.push_back(item);
        }
    }
    
    return answer;
}


template <typename T>
ship_guns::p_response_t ship_guns::partial_response (T const & mount)
{
    p_response_t item;
    item.group = mount.class_id;
    item.group_name = armament_links::filtered("/armament/guns?sort=caliber,in_service&group=caliber", mount.class_ru.value_or(""), mount.class_id);
    if (mount.caliber)
        item.compare = -std::floor((std::log(*mount.caliber + 1.) + 0.5) / 0.3);
    else
        item.compare = 0;
    
    item.data += "x";
    add_value(item.data, mount.gun_count);
    item.data += " ";
    if (mount.caliber)
        item.data.append(to_string_10(*mount.caliber) + "мм");
    if (mount.length)
        item.data.append("/")
                 .append(to_string_10(*mount.length));
    item.data.append("  ");
    item.data += mount.gun_ru.value_or("  ") + new_line;
    if (mount.mount_ru || mount.angle)
    {
        item.data += "&emsp;(";
        if (mount.mount_ru)
            item.data += *mount.mount_ru + "  ";
        if (mount.angle)
            item.data += to_string_10(*mount.angle) + "°";
        item.data += ")";
    }
    return item;
}


