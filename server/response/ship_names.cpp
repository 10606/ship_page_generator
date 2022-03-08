#include "ship_names.h"

#include "ship_info.h"
#include "date_to_str.h"


ship_names::response_t ship_names::response (std::vector <std::pair <int, std::chrono::year_month_day> > ship_year)
{
    response_t answer = 
    {
        std::string(table.begin) + std::string(table.new_column),
        std::vector <uint8_t> (ship_year.size())
    };

    for (size_t i = 0; i != ship_year.size(); ++i)
    {
        if (i != 0)
            answer.row += table.new_column;
        
        std::vector <ship_requests::ship_info_t::list> ship =
            database->ship_info.get_list("where ship_list.id = " + std::to_string(ship_year[i].first));
        if (ship.empty())
            continue;
        
        answer.row += ship[0].ship_ru.value_or("");
        if (ship[0].type_ru)
            answer.row += std::string(table.new_line) + "(тип " + *(ship[0].type_ru) + ")";
        answer.row += std::string(table.new_line) + to_string(ship_year[i].second);

        size_t modernizations =
            database->ship_event.count("where ship_id = " + std::to_string(ship_year[i].first) + 
                                       " and  date_from <= " + to_string_sql(ship_year[i].second) +
                                       " and  date_to > " + to_string_sql(ship_year[i].second) +
                                       " and  class_id = 0");
        answer.modernization[i] = modernizations;
        if (modernizations)
            answer.row += std::string(table.new_line) + "на модернизации";
        if (ship[0].commissioned && ship_year[i].second < *ship[0].commissioned)
            answer.row += std::string(table.new_line) + "еще не введен в строй";
        if (ship[0].sunk_date && ship_year[i].second > *ship[0].sunk_date)
            answer.row += std::string(table.new_line) + "потоплен";
    }

    answer.row += std::string(table.end);
    return answer;
}
