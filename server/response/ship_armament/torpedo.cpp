#include "torpedo.h"

#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"

std::vector <ship_torpedo_tubes::response_t> ship_torpedo_tubes::response (int id, std::chrono::year_month_day date)
{
    std::vector <ship_requests::ship_armament_t::torpedo_tubes> torpedo_tubes =
        database->ship_armament.get_torpedo_tubes(where("ship_torpedo_tubes", id, date) + 
                                                  "order by (torpedo_tubes.class_id, -caliber, torpedo_tubes.id)");

    std::vector <response_t> answer;
    answer.reserve(torpedo_tubes.size());

    for (auto tube : torpedo_tubes)
    {
        response_t item;
        item.group = 0;
        item.compare = tube.class_id;;
        item.group_name = "торпедный аппарат";
        
        item.data += std::to_string(tube.mount_count) + "x" + std::to_string(tube.tubes_count) + " ";
        item.data += (tube.caliber? (to_string_10(*tube.caliber) + "мм  ") : "  ");
        item.data += tube.tube_ru.value_or("  ") + new_line;

        answer.push_back(item);
    }
    
    return answer;
}
