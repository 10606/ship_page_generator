#include "searchers.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "armament_links.h"


ship_searchers::ship_searchers (ship_requests * _database, std::string_view _new_line) :
    database(_database),
    new_line(_new_line)
{
    std::vector <ship_searchers_t> searcher_list =
        database->ship_armament_lt.get_searchers("");

    for (ship_searchers_t & searcher : searcher_list)
        ship_searchers_list[searcher.ship_id].push_back(std::move(searcher));

    // sorting
    {
        std::vector <searcher_t> mounts_list =
            database->armament_info.get_searchers();
        std::unordered_map <int, searcher_t> searchers_full;
        for (searcher_t & searcher : mounts_list)
        {
            int searcher_id = searcher.id;
            searchers.insert({searcher_id, partial_response(searcher)});
            searchers_full.insert({searcher_id, std::move(searcher)});
        }
        
        auto torpedo_order = 
            [&searchers_full] (ship_searchers_t const & a, ship_searchers_t const & b) -> bool
            {
                // class_id, searcher_id
                searcher_t const & a_info = searchers_full.at(a.searcher_id);
                searcher_t const & b_info = searchers_full.at(b.searcher_id);
                
                std::strong_ordering class_cmp = a_info.class_id <=> b_info.class_id;
                if (class_cmp != std::strong_ordering::equal)
                    return std::is_lt(class_cmp);
                    
                return a.searcher_id < b.searcher_id;
            };
        
        for (auto & item : ship_searchers_list)
            std::sort(item.second.begin(), item.second.end(), torpedo_order);
    }
}

std::vector <ship_searchers::response_t> ship_searchers::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t> answer;

    std::unordered_map <int, std::vector <ship_searchers_t> > :: const_iterator it = ship_searchers_list.find(id);
    if (it == ship_searchers_list.end())
        return answer;
    for (ship_searchers_t const & searcher : it->second)
    {
        if (between(searcher.date_from, date, searcher.date_to))
        {
            std::unordered_map <int, p_response_t> :: const_iterator searcher_it = searchers.find(searcher.searcher_id);
            response_t item = (searcher_it != searchers.end())? searcher_it->second : response_t();
            item.data_begin = std::to_string(searcher.count) + " ";
            answer.push_back(item);
        }
    }
    
    return answer;
}

ship_searchers::p_response_t ship_searchers::partial_response (searcher_t const & searcher)
{
    p_response_t item;
    item.group = searcher.class_id;
    item.group_name = armament_links::filtered("/armament/searcher?sort=in_service", searcher.class_ru.value_or(""), searcher.class_id);
    item.compare = 0;
    
    item.data += searcher.searcher_ru.value_or("  ");
    return item;
}
    
    
