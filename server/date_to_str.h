#ifndef DATE_TO_STR_H
#define DATE_TO_STR_H

#include <string>
#include <chrono>
#include <charconv>

std::string to_string (std::chrono::year_month_day const & value);
std::string to_string_sql (std::chrono::year_month_day const & value);
std::string to_string_10 (double value);

inline void add_value (std::string & answer, uint32_t value)
{
    char value_char[16];
    std::to_chars_result res = std::to_chars(std::begin(value_char), std::end(value_char), value);
    answer.append(value_char, res.ptr);
}

std::string where
(
    std::string const & table_name,
    int ship_id,
    std::chrono::year_month_day const & date
);

#endif

