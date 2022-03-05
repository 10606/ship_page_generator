#include "guns.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"

std::vector <ship_guns::response_t> ship_guns::response (int id, std::chrono::year_month_day date)
{
    std::vector <ship_requests::ship_armament_t::guns> guns =
        database->ship_armament.get_guns("where ship_guns.ship_id = " + std::to_string(id) + 
                                        " and  ship_guns.date_from <= " + to_string_sql(date) +
                                        " and  ship_guns.date_to > " + to_string_sql(date) +
                                        "order by (gun_list.class_id, -caliber, gun_list.id)");

    std::vector <response_t> answer;
    answer.reserve(guns.size());

    for (auto gun : guns)
    {
        response_t item;
        item.group = gun.class_id;
        item.group_name = gun.class_ru.value_or("");
        if (gun.caliber)
            item.compare = std::numeric_limits <size_t> ::max() - 
                static_cast <size_t> ((std::log(*gun.caliber + 1.) + 0.5) / 0.3);
        else
            item.compare = 0;
        
        item.data += std::to_string(gun.mount_count) + "x" + std::to_string(gun.gun_count) + " ";
        item.data += (gun.caliber? (to_string_10(*gun.caliber) + "мм  ") : "  ");
        item.data += gun.gun_ru.value_or("  ") + new_line;
        if (gun.mount_ru || gun.angle)
        {
            item.data += "&emsp;(";
            if (gun.mount_ru)
                item.data += *gun.mount_ru + "  ";
            if (gun.angle)
                item.data += to_string_10(*gun.angle) + "°";
            item.data += ")";
        }

        answer.push_back(item);
    }
    
    return answer;
}
