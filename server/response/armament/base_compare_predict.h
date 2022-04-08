#ifndef BASE_COMPARE_PREDICT_H
#define BASE_COMPARE_PREDICT_H

#include "group_and_sorting.h"
#include <charconv>
#include <chrono>
#include <functional>
#include <optional>
#include <set>

template <typename T>
std::partial_ordering 
compare_null_last 
(
    std::optional <T> const & a, 
    std::optional <T> const & b
)
{
    if (!a && !b)
        return std::partial_ordering::equivalent;
    if (a && b)
        return *a <=> *b;
    if (!a)
        return std::partial_ordering::greater;
    if (!b)
        return std::partial_ordering::less;
    return std::partial_ordering::equivalent; // just for aviod warning
}


std::partial_ordering 
compare_date_10th 
(
    std::optional <std::chrono::year_month_day> const & a, 
    std::optional <std::chrono::year_month_day> const & b
)
{
    if (!a && !b)
        return std::partial_ordering::equivalent;
    if (a && b)
        return (static_cast <int> (a->year()) / 10) <=> (static_cast <int> (b->year()) / 10);
    if (!a)
        return std::partial_ordering::greater;
    if (!b)
        return std::partial_ordering::less;
    return std::partial_ordering::equivalent; // just for aviod warning
}


template <typename T>
struct year_filter
{
    year_filter (std::span <std::string_view const> values)
    {
        for (std::string_view year : values)
        {
            std::optional <int> parsed = parse_10th(year);
            if (parsed)
                years.insert(*parsed);
        }
    }

    bool operator () (T const & value)
    {
        if (!value.in_service)
            return 0;
        int in_service = static_cast <int> (value.in_service->year());
        in_service -= in_service % 10;
        return years.find(in_service) != years.end();
    }
    
    static std::optional <int> parse_10th (std::string_view year)
    {
        if (year.size() != 2 &&
            year.size() != 4)
            return std::nullopt;
        if (year.back() != 'x')
            return std::nullopt;
        int answer = 0;
        for (size_t i = 0; i != year.size() - 1; ++i)
        {
            if (!std::isdigit(year[i]))
                return std::nullopt;
            answer = answer * 10 + (year[i] - '0');
        }
        if (year.size() == 2)
            answer += 190;
        return answer * 10;
    }
    
private:
    std::set <int> years;
};


inline std::optional <double> parse_double (std::string_view value)
{
    if (value.empty())
        return std::nullopt;
    double answer;
    std::from_chars_result res = std::from_chars(value.begin(), value.end(), answer);
    if (res.ec == std::errc())
        return answer;
    else
        return std::nullopt;
}


template <typename T>
struct caliber_filter
{
    caliber_filter (std::span <std::string_view const> values)
    {
        for (std::string_view year : values)
        {
            std::optional <double> parsed = parse_double(year);
            if (parsed)
                years.insert(*parsed);
        }
    }

    bool operator () (T const & value)
    {
        if (!value.caliber)
            return 0;
        return years.find(*value.caliber) != years.end();
    }

private:
    std::set <double> years;
};   

#endif

