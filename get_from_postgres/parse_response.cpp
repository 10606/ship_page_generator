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
    uint8_t day   = static_cast <unsigned> (value.day()) % 100;
    uint8_t month = static_cast <unsigned> (value.month()) % 100;
    std::string answer
    {
        static_cast <char> ('0' + day / 10),
        static_cast <char> ('0' + day % 10),
        '.',
        static_cast <char> ('0' + month / 10),
        static_cast <char> ('0' + month % 10),
        '.'
    };
    answer.append( std::to_string(static_cast <int> (value.year())));
    return answer;
}

