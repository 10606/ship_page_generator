#include "searchers.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "armament_links.h"


ship_searchers::ship_searchers (ship_requests * database, std::string_view _new_line) :
    new_line(_new_line)
{
    std::vector <searcher_t> searchers_full =
        database->armament_info.get_searchers();
    std::unordered_map <int, size_t> searchers_index;
    for (searcher_t & searcher : searchers_full)
    {
        int searcher_id = searcher.id;
        searchers_index.insert({searcher_id, searchers.size()});
        searchers.push_back(partial_response(searcher));
    }
    
    std::vector <ship_searchers_t> searcher_list =
        database->ship_armament_lt.get_searchers("");
    for (ship_searchers_t & searcher : searcher_list)
    {
        std::unordered_map <int, size_t> ::iterator it = searchers_index.find(searcher.searcher_id);
        if (it != searchers_index.end())
            ship_searchers_list[searcher.ship_id].emplace_back(it->second, searcher);
    }

    // sorting
    {
        auto torpedo_order = 
            [&searchers_full] (ship_searchers_lt const & a, ship_searchers_lt const & b) -> bool
            {
                // class_id, searcher_id
                searcher_t const & a_info = searchers_full[a.searcher_id];
                searcher_t const & b_info = searchers_full[b.searcher_id];
                
                std::strong_ordering class_cmp = a_info.class_id <=> b_info.class_id;
                if (class_cmp != std::strong_ordering::equal)
                    return std::is_lt(class_cmp);
                    
                return a_info.id < b_info.id;
            };
        
        for (auto & item : ship_searchers_list)
            std::sort(item.second.begin(), item.second.end(), torpedo_order);
    }
}

std::vector <ship_searchers::response_t> ship_searchers::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_searchers_lt> > :: const_iterator it = ship_searchers_list.find(id);
    if (it == ship_searchers_list.end())
        return answer;
    for (ship_searchers_lt const & searcher : it->second)
    {
        if (between(searcher.date_from, date, searcher.date_to))
        {
            response_t item = searchers[searcher.searcher_id];
            add_value(item.data_begin, searcher.count);
            answer.push_back(item);
        }
    }
    
    return answer;
}

ship_searchers::p_response_t ship_searchers::partial_response (searcher_t const & searcher)
{
    p_response_t item;
    item.group = searcher.class_id;
    item.group_name = armament_links::filtered("/armament/searcher?group=power&sort=in_service", searcher.class_ru.value_or(""), searcher.class_id);
    item.compare = 0;
    
    item.data += " ";
    item.data += searcher.searcher_ru.value_or("  ");
    return item;
}
    
    
