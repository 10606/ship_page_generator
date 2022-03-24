#include "throwers.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"

std::vector <ship_throwers::response_t> ship_throwers::response (int id, std::chrono::year_month_day date)
{
    std::vector <ship_requests::ship_armament_t::throwers> throwers =
        database->ship_armament.get_throwers(where("ship_throwers", id, date) +
                                             "order by (gun_class.id, -caliber, throwers.id)");

    std::vector <response_t> answer;
    answer.reserve(throwers.size());

    for (auto thrower : throwers)
    {
        response_t item;
        item.group = 0;
        item.group_name = "противолодочное вооружение";
        item.compare = 0;
        
        item.data += std::to_string(thrower.mount_count) + "x" + std::to_string(thrower.tubes_count) + " ";
        item.data += (thrower.caliber? (to_string_10(*thrower.caliber) + "мм  ") : "  ");
        item.data += (thrower.class_ru? *thrower.class_ru + "  ": "");
        item.data += thrower.thrower_ru.value_or("  ");

        answer.push_back(item);
    }
    
    return answer;
}

