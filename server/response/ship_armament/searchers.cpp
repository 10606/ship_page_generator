#include "searchers.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"

std::vector <ship_searchers::response_t> ship_searchers::response (int id, std::chrono::year_month_day date)
{
    std::vector <ship_requests::ship_armament_t::searchers> searchers =
        database->ship_armament.get_searchers(where("ship_searchers", id, date) +
                                              "order by (gun_class.id, searchers.id)");

    std::vector <response_t> answer;
    answer.reserve(searchers.size());

    for (auto searcher : searchers)
    {
        response_t item;
        item.group = searcher.class_id;
        item.group_name = searcher.class_ru.value_or("");
        item.compare = 0;
        
        item.data += std::to_string(searcher.count) + " ";
        item.data += searcher.searcher_ru.value_or("  ");

        answer.push_back(item);
    }
    
    return answer;
}

