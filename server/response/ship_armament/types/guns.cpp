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


ship_guns::ship_guns (ship_requests & database, std::string_view _new_line) :
    ship_guns_list_segmented(),
    new_line(_new_line)
{
    typedef ship_requests::armament_info_t::classes classes;
    std::vector <classes> gun_class_list =
        database.armament_info.get_classes("");
    for (classes const & cur_class : gun_class_list)
        cache_class_names.insert
        (
            {
                cur_class.class_id,
                armament_links::filtered("/armament/guns?sort=caliber,in_service&group=caliber", cur_class.class_ru.value_or(""), cur_class.class_id)
            }
        );

    std::unordered_map <int, std::vector <ship_items_lt> > ship_guns_list;
                
    fill_data_structures
    <
        ship_guns,
        ship_guns_t,
        mount_t,
        &ship_guns::mounts,
        &ship_guns_t::mount_id
    >
    (
        *this, 
        database.armament_info.get_mount(),
        database.ship_armament_lt.get_guns(""),
        &ship_guns_list,
        
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

    auto mount_info_transform = 
        [] (ship_items_lt value) -> segment_data
        {
            return segment_data
                   {
                       value.date_from,
                       value.date_to,
                       mount_info_t
                       {
                           value.mount_id,
                           value.mount_count
                       }
                   };
        };

    for (decltype(ship_guns_list)::value_type value : ship_guns_list)
    {
        std::vector <segment_data> cur;
        std::transform(value.second.begin(), value.second.end(), std::back_inserter <decltype(cur)> (cur), mount_info_transform);
        ship_guns_list_segmented.insert({value.first, std::move(cur)});
    }
}


std::vector <ship_guns::response_t, allocator_for_temp <ship_guns::response_t> >
ship_guns::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t, allocator_for_temp <response_t> > answer;

    std::unordered_map <int, segments_t> :: const_iterator it = ship_guns_list_segmented.find(id);
    if (it == ship_guns_list_segmented.end())
        return answer;
    segments_t::value_list const & values = it->second.get(date);
    answer.reserve(values.size());
    for (mount_info_t const & i : values)
    {
        answer.emplace_back(mounts[i.mount_id]);
        add_value(answer.back().data_begin, i.mount_count);
    }
    
    return answer;
}


ship_guns::p_response_t ship_guns::partial_response (mount_t const & mount)
{
    p_response_t item;
    item.group = mount.class_id;

    std::unordered_map <int, std::string> ::iterator it = cache_class_names.find(mount.class_id);
    if (it != cache_class_names.end())
        item.group_name = it->second;
    if (mount.caliber)
        item.compare = -std::floor((std::log(*mount.caliber + 1.) + 0.5) / 0.3);
    else
        item.compare = 0;
    
    item.data.append("x<b>");
    add_value(item.data, mount.gun_count);
    item.data += " ";
    if (mount.caliber)
        item.data.append(to_string_10(*mount.caliber))
                 .append("мм");
    item.data.append("</b>");
    if (mount.length)
        item.data.append("/")
                 .append(to_string_10(*mount.length));
    item.data.append(" <b>")
             .append(mount.gun_ru.value_or("  "))
             .append("</b>")
             .append(new_line);
    if (mount.mount_ru || mount.angle)
    {
        item.data += "&emsp;(";
        if (mount.mount_ru)
            item.data += *mount.mount_ru + " ";
        if (mount.angle)
            item.data += to_string_10(*mount.angle) + "°";
        item.data += ")";
    }
    return item;
}


