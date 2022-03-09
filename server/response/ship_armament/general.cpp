#include "general.h"

#include <vector>
#include <chrono>
#include <cmath>
#include "ship_requests.h"
#include "ship_info.h"
#include "date_to_str.h"

std::vector <ship_general::response_t> ship_general::response (int id, std::chrono::year_month_day date)
{
    std::vector <ship_requests::ship_info_t::general> general =
        database->ship_info.get_general(where("general", id, date));

    std::vector <response_t> answer;
    answer.reserve(5);

    if (general.empty())
        return answer;
    
    {
        response_t item;
        item.group = 0;
        item.group_name = "характеристики";
        item.compare = 0;
        
        if (general[0].displacement_standart)
            item.data += to_string_10(*general[0].displacement_standart) + "т";
        if (general[0].displacement_standart || general[0].displacement_full)
            item.data += " .. ";
        if (general[0].displacement_full)
            item.data += to_string_10(*general[0].displacement_full) + "т";
        answer.push_back(item);
    }
    
    {
        response_t item;
        item.group = 0;
        item.group_name = "характеристики";
        item.compare = 1;
        
        if (general[0].length || general[0].width || general[0].draft)
        {
            item.data += (general[0].length? to_string_10(*general[0].length) + "м" : "??");
            item.data += " x ";
            item.data += (general[0].width?  to_string_10(*general[0].width)  + "м" : "??");
            item.data += " x ";
            item.data += (general[0].draft?  to_string_10(*general[0].draft)  + "м" : "??");
        }
        answer.push_back(item);
    }
    
    {
        response_t item;
        item.group = 0;
        item.group_name = "характеристики";
        item.compare = 2;
        
        if (general[0].crew)
            item.data += std::to_string(*general[0].crew) + "чел";
        answer.push_back(item);
    }
    
    {
        response_t item;
        item.group = 0;
        item.group_name = "характеристики";
        item.compare = 3;
        
        if (general[0].speed_max)
            item.data += to_string_10(*general[0].speed_max) + "узлов";
        answer.push_back(item);
    }
    
    {
        response_t item;
        item.group = 0;
        item.group_name = "характеристики";
        item.compare = 4;
        
        if (general[0].speed_cruise)
            item.data += to_string_10(*general[0].speed_cruise) + "узлов ";
        if (general[0].range)
            item.data += to_string_10(*general[0].range) + "км";
        answer.push_back(item);
    }
    
    return answer;
}

