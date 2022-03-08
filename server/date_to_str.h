#ifndef DATE_TO_STR_H
#define DATE_TO_STR_H

#include <string>
#include <chrono>

std::string to_string (std::chrono::year_month_day const & value);
std::string to_string_sql (std::chrono::year_month_day const & value);
std::string to_string_10 (double value);

std::string where
(
    std::string const & table_name,
    int ship_id,
    std::chrono::year_month_day const & date
);

#endif

