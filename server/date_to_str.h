#ifndef DATE_TO_STR_H
#define DATE_TO_STR_H

#include <string>
#include <chrono>

std::string to_string (std::chrono::year_month_day const & value);
std::string to_string_sql (std::chrono::year_month_day const & value);
std::string to_string_10 (double value);

#endif

