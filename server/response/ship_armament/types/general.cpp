#include "general.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "common.h"
#include "ship_requests.h"
#include "ship_info.h"
#include "date_to_str.h"
#include "ship_armament_utils.h"


ship_general::ship_general (ship_requests * database, std::string_view _new_line) :
    new_line(_new_line)
{
    sunk_dates_t const & sunk_date = sunk_dates();
    std::vector <ship_requests::ship_info_t::general> general_list =
        database->ship_info.get_general("");

    for (general_t & general : general_list)
    {
        sunk_dates_t::const_iterator sunk = sunk_date.find(general.ship_id);
        if (sunk != sunk_date.end() && sunk->second == general.date_to)
            general.date_to = std::nullopt;
        ship_general_list[general.ship_id].push_back(partial_response(std::move(general)));
    }
}


std::vector <ship_general::response_t> ship_general::response (int id, std::chrono::year_month_day date) const
{
    std::unordered_map <int, std::vector <response_with_time_label> > :: const_iterator it = ship_general_list.find(id);
    if (it == ship_general_list.end())
        return std::vector <response_t> ();
    for (response_with_time_label const & response_with_time : it->second)
    {
        if (between(response_with_time.date_from, date, response_with_time.date_to))
        {
            std::vector <response_t> answer;
            answer.reserve(response_with_time.answer.size());
            for (p_response_t const & item : response_with_time.answer)
                answer.push_back(item);
            return answer;
        }
    }
    
    return std::vector <response_t> ();
}


ship_general::response_with_time_label ship_general::partial_response (general_t const & general)
{
    std::vector <p_response_t> answer;
    answer.reserve(5);

    {
        p_response_t item;
        item.group = 0;
        item.compare = 0;
        item.group_name = "характеристики";
        
        item.data.append("<b>");
        if (general.displacement_standart)
            item.data += to_string_10(*general.displacement_standart) + "т";
        if (general.displacement_standart || general.displacement_full)
            item.data += " .. ";
        if (general.displacement_full)
            item.data += to_string_10(*general.displacement_full) + "т";
        item.data.append("</b>");
        answer.push_back(item);
    }
    
    {
        p_response_t item;
        item.group = 0;
        item.group_name = "характеристики";
        item.compare = 1;
        
        if (general.length || general.width || general.draft)
        {
            item.data += (general.length? to_string_10(*general.length) + "м" : "??");
            item.data += " x ";
            item.data += (general.width?  to_string_10(*general.width)  + "м" : "??");
            item.data += " x ";
            item.data += (general.draft?  to_string_10(*general.draft)  + "м" : "??");
        }
        answer.push_back(item);
    }
    
    {
        p_response_t item;
        item.group = 0;
        item.group_name = "характеристики";
        item.compare = 2;
        
        if (general.crew)
        {
            add_value(item.data, *general.crew);
            item.data += "чел";
        }
        answer.push_back(item);
    }
    
    {
        p_response_t item;
        item.group = 0;
        item.group_name = "характеристики";
        item.compare = 3;
        
        item.data.append("<b>");
        if (general.speed_max)
            item.data += to_string_10(*general.speed_max) + "узлов";
        item.data.append("</b>");
        answer.push_back(item);
    }
    
    {
        p_response_t item;
        item.group = 0;
        item.group_name = "характеристики";
        item.compare = 4;
        
        if (general.speed_cruise)
            item.data += to_string_10(*general.speed_cruise) + "узлов ";
        if (general.range)
            item.data += to_string_10(*general.range) + "км";
        answer.push_back(item);
    }
    
    return response_with_time_label{std::move(answer), general.date_from, general.date_to};
}

