#include "date_to_str.h"

std::string to_string_sql (std::chrono::year_month_day const & value)
{
    return "\'" +
           std::to_string(static_cast <int> (value.year())) + "-" + 
           std::to_string(static_cast <unsigned> (value.month())) + "-" + 
           std::to_string(static_cast <unsigned> (value.day())) + "\'";
}

std::string to_string_10 (double value)
{
    size_t tmp = value * 10;
    if (tmp % 10 == 0)
        return std::to_string(tmp / 10);
    else
    {
        std::string answer = std::to_string(tmp / 10);
        answer.push_back('.');
        answer.push_back(tmp % 10 + '0');
        return answer;
    }
}

std::string where
(
    std::string const & table_name,
    int ship_id,
    std::chrono::year_month_day const & date
)
{
    return  std::string("where ") + 
            table_name + ".ship_id = " + std::to_string(ship_id) + 
            " and (" + table_name + ".date_from <= " + to_string_sql(date) + " or " + table_name + ".date_from is null)" +
            " and (" + table_name + ".date_to > " + to_string_sql(date) + " or " + table_name + ".date_to is null)";
}


