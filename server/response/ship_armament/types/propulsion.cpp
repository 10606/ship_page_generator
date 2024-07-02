#include "propulsion.h"

#include "ship_requests.h"
#include "ship_armament.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"
#include "armament_links.h"
#include "common.h"


ship_propulsion::ship_propulsion (ship_requests * database, std::string_view _new_line) :
    new_line(_new_line),
    group_name("силовая установка")
{
    ship_requests::propulsion_t::context storage(database->ship_propulsion);
    fill_data_structures
    <
        ship_propulsion,
        ship_propulsions_t,
        propulsion_t,
        &ship_propulsion::propulsions,
        &ship_propulsions_t::propulsion_id
    >
    (
        *this, 
        database->ship_propulsion.get_propulsion(storage),
        database->ship_propulsion.get_ship_propulsion(),
        &ship_propulsions_list,
        
        [] (std::vector <propulsion_t> const & propulsions_full, std::vector <size_t> const & old_index)
        {
            return
            [&propulsions_full, &old_index] (ship_items_lt const & a, ship_items_lt const & b) -> bool
            {
                // class_id, propulsion_id
                propulsion_t const & a_info = propulsions_full[old_index[a.propulsion_id]];
                propulsion_t const & b_info = propulsions_full[old_index[b.propulsion_id]];
                
                return a_info->id < b_info->id;
            };
        },
        
        std::ref(storage)
    );
}

std::vector <ship_propulsion::response_t, allocator_for_temp <ship_propulsion::response_t> >
ship_propulsion::response (int id, std::chrono::year_month_day date) const
{
    std::vector <response_t, allocator_for_temp <response_t> > answer;

    std::unordered_map <int, std::vector <ship_items_lt> > :: const_iterator it = ship_propulsions_list.find(id);
    if (it == ship_propulsions_list.end())
        return answer;
    size_t i = 0;
    for (ship_items_lt const & propulsion : it->second)
    {
        if (between(propulsion.date_from, date, propulsion.date_to))
        {
            answer.emplace_back(propulsions[propulsion.propulsion_id]);
            if (i != 0)
                answer.back().data_begin.append("<br>");
            if (propulsion.count > 1)
            {
                answer.back().data_begin.append("<b>");
                add_value(answer.back().data_begin, propulsion.count);
                declension(answer.back().data_begin, propulsion.count, {" установка", " установки", " установок"});
                answer.back().data_begin.append("</b>:<br><br>");
            }
            answer.back().group_name = group_name;
            i++;
        }
    }
    
    return answer;
}

ship_propulsion::p_response_t ship_propulsion::partial_response (propulsion_t const & propulsion, ship_requests::propulsion_t::context const & storage)
{
    p_response_t item;
    
    ship_requests::propulsion_t::print_context print
    {
        .tab = "&nbsp;&nbsp;&nbsp;&nbsp;", 
        .new_line = "<br>", 
        .bold_begin = "<b>", 
        .bold_end = "</b>"
    };
    
    item.data += propulsion->description(print, storage);
    return item;
}
    
    
