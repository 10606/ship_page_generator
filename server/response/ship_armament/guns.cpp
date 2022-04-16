#include "guns.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "nested_segments.h"
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"


ship_guns::ship_guns (ship_requests * _database, std::string_view _new_line) :
    ship_guns_list(),
    database(_database),
    new_line(_new_line)
{
    std::vector <ship_guns_t> guns_list =
        database->ship_armament_lt.get_guns("");

    for (ship_guns_t & gun : guns_list)
        ship_guns_list[gun.ship_id].push_back(std::move(gun));

    {
        std::vector <mount_t> mounts_list =
            database->armament_info.get_mount();
        std::unordered_map <int, mount_t> mounts_full;
        for (mount_t & mount : mounts_list)
        {
            int mount_id = mount.id;
            mounts.insert({mount_id, partial_response(mount)});
            mounts_full.insert({mount_id, std::move(mount)});
        }

        auto guns_order = 
            [&mounts_full] (ship_guns_t const & a, ship_guns_t const & b) -> bool
            {
                // class_id, -caliber, gun_id, gun_count, mount_id
                mount_t const & a_info = mounts_full.at(a.mount_id);
                mount_t const & b_info = mounts_full.at(b.mount_id);
                
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
                    return std::is_lt(cnt_cmp);
                    
                return a.mount_id < b.mount_id;
            };
        
        for (auto & item : ship_guns_list)
            std::sort(item.second.begin(), item.second.end(), guns_order);
    }
}


std::vector <ship_guns::response_t> ship_guns::response (int id, std::chrono::year_month_day date)
{
    std::vector <response_t> answer;
    for (ship_guns_t const & value : ship_guns_list[id])
    {
        if ((!value.date_from || *value.date_from <= date) &&
            (!value.date_to   || date < *value.date_to))
        {
            response_t item = mounts[value.mount_id];
            item.data = std::to_string(value.mount_count) + item.data;
            answer.push_back(item);
        }
    }
    
    return answer;
}


template <typename T>
ship_guns::response_t ship_guns::partial_response (T const & mount)
{
    response_t item;
    item.group = mount.class_id;
    item.group_name = mount.class_ru.value_or("");
    if (mount.caliber)
        item.compare = -std::floor((std::log(*mount.caliber + 1.) + 0.5) / 0.3);
    else
        item.compare = 0;
    
    item.data += "x" + std::to_string(mount.gun_count) + " ";
    item.data += (mount.caliber? (to_string_10(*mount.caliber) + "мм  ") : "  ");
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


