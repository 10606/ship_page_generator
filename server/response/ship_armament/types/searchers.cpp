#include "searchers.h"

#include <vector>
#include <chrono>
#include <cmath>
#include <set>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "armament_links.h"
#include "common.h"


ship_searchers::ship_searchers (ship_requests * database, std::string_view _new_line) :
    new_line(_new_line)
{
    fill_data_structures
    <
        ship_searchers,
        ship_searchers_t,
        ship_searchers_lt,
        searcher_t,
        &ship_searchers::searchers,
        &ship_searchers::ship_searchers_list,
        &ship_searchers_t::searcher_id
    >
    (
        *this, 
        database->armament_info.get_searchers(),
        database->ship_armament_lt.get_searchers(""),
        
        [] (std::vector <searcher_t> const & searchers_full, std::vector <size_t> const & old_index)
        {
            return
            [&searchers_full, &old_index] (ship_searchers_lt const & a, ship_searchers_lt const & b) -> bool
            {
                // class_id, searcher_id
                searcher_t const & a_info = searchers_full[old_index[a.searcher_id]];
                searcher_t const & b_info = searchers_full[old_index[b.searcher_id]];
                
                std::strong_ordering class_cmp = a_info.class_id <=> b_info.class_id;
                if (class_cmp != std::strong_ordering::equal)
                    return std::is_lt(class_cmp);
                    
                return a_info.id < b_info.id;
            };
        }
    );
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
    
    
