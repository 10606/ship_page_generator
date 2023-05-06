#ifndef DATE_TO_STR_H
#define DATE_TO_STR_H

#include <string>
#include <chrono>
#include <charconv>
#include "simple_string.h"

std::string to_string (std::chrono::year_month_day const & value);
std::string to_string_sql (std::chrono::year_month_day const & value);
std::string to_string_10 (double value);

inline void add_value (std::string & answer, uint32_t value)
{
    char value_char[16];
    std::to_chars_result res = std::to_chars(std::begin(value_char), std::end(value_char), value);
    answer.append(value_char, res.ptr);
}

inline void add_value (simple_string & answer, uint32_t value)
{
    char value_char[16];
    std::to_chars_result res = std::to_chars(std::begin(value_char), std::end(value_char), value);
    answer.append(std::string_view(value_char, res.ptr));
}

template <size_t n>
inline char * add_value (char (& answer) [n], uint32_t value)
{
    char * ret = std::to_chars(std::begin(answer), std::end(answer), value).ptr;
    *ret = '\0';
    return ret;
}

std::string where
(
    std::string const & table_name,
    int ship_id,
    std::chrono::year_month_day const & date
);


struct declension_t
{
    std::string_view one;
    std::string_view two_four;
    std::string_view other;
};

inline void declension (std::string & answer, uint32_t count, declension_t values)
{
    if ((count % 10 == 1) && (count != 11))
        answer.append(values.one);
    else if ((count % 10 <= 4) && ((count / 10) % 10 != 1))
        answer.append(values.two_four); // 12 "ступеней / котлов" все-таки...
    else
        answer.append(values.other);
}


#endif

