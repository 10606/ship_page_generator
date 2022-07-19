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


ship_guns::ship_guns (ship_requests * database, std::string_view _new_line) :
    ship_guns_list(),
    new_line(_new_line)
{
    std::vector <mount_t> mounts_full =
        database->armament_info.get_mount();
    std::vector <ship_guns_t> guns_list =
        database->ship_armament_lt.get_guns("");
    
    std::set <int> used; // add only used
    for (ship_guns_t const & gun : guns_list)
        used.insert(gun.mount_id);

    std::unordered_map <int, size_t> mounts_index;
    mounts.reserve(used.size());
    for (mount_t & mount : mounts_full)
    {
        int mount_id = mount.id;
        if (used.find(mount_id) == used.end())
            continue;
        mounts_index.insert({mount_id, mounts.size()});
        mounts.push_back(partial_response(mount));
    }

    for (ship_guns_t & gun : guns_list)
    {
        std::unordered_map <int, size_t> ::iterator it = mounts_index.find(gun.mount_id);
        if (it != mounts_index.end())
            ship_guns_list[gun.ship_id].emplace_back(it->second, gun);
    }

    // sorting
    {
        auto guns_order = 
            [&mounts_full] (ship_guns_lt const & a, ship_guns_lt const & b) -> bool
            {
                // class_id, -caliber, gun_id, -gun_count, mount_id
                mount_t const & a_info = mounts_full[a.mount_id];
                mount_t const & b_info = mounts_full[b.mount_id];
                
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
        
        for (auto & item : ship_guns_list)
            std::sort(item.second.begin(), item.second.end(), guns_order);
    }
}


std::vector <ship_guns::response_t> ship_guns::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_guns_lt> > :: const_iterator it = ship_guns_list.find(id);
    if (it == ship_guns_list.end())
        return answer;
    for (ship_guns_lt const & value : it->second)
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


