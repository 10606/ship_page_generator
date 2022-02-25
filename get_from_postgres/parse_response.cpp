#include "ship_requests.h"
#include "ship_armament.h"

#include <string>

std::chrono::year_month_day get_date (std::string const & value)
{
    int year;
    int month;
    int day;
    sscanf(value.c_str(), "%d-%d-%d", &year, &month, &day);
    return std::chrono::year_month_day(std::chrono::year(year), std::chrono::month(month), std::chrono::day(day));
}

std::string to_string (std::chrono::year_month_day const & value)
{
    return std::to_string(static_cast <unsigned> (value.day())) + "." +
           std::to_string(static_cast <unsigned> (value.month())) + "." +
           std::to_string(static_cast <int> (value.year()));
}

