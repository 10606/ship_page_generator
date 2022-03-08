#include "catapult.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"

std::vector <ship_catapult::response_t> ship_catapult::response (int id, std::chrono::year_month_day date)
{
    std::vector <ship_requests::ship_armament_t::catapult> catapults =
        database->ship_armament.get_catapult(where("ship_catapult", id, date) +
                                             "order by (catapult_class.id, catapult.id)");

    std::vector <response_t> answer;
    answer.reserve(catapults.size());

    for (auto catapult : catapults)
    {
        response_t item;
        item.group = 0;
        item.compare = catapult.class_id;;
        item.group_name = "катапульта";
        
        item.data += std::to_string(catapult.count) + " ";
        item.data += catapult.catapult_ru.value_or("  ");

        answer.push_back(item);
    }
    
    return answer;
}

