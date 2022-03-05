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
        return std::to_string(tmp / 10) + '.' + std::to_string(tmp % 10);
}


