#ifndef NESTED_SEGMENTS_H
#define NESTED_SEGMENTS_H


#include <chrono>
#include <vector>
#include <optional>

struct segment
{
    segment
    (
        std::optional <std::chrono::year_month_day> _begin,
        std::optional <std::chrono::year_month_day> _end,
        size_t _index
    ) :
        begin(std::move(_begin)),
        end(std::move(_end)),
        index(_index)
    {}
    
    std::optional <std::chrono::year_month_day> begin;
    std::optional <std::chrono::year_month_day> end;
    size_t index;
};

std::vector <std::vector <size_t> >
nested_segments 
(std::vector <segment> values);


template <typename T>
std::strong_ordering compare_null
(
    std::optional <T> const & a, std::optional <T> const & b, 
    std::strong_ordering not_a, std::strong_ordering not_b
)
{
    if (!a && !b)
        return std::strong_ordering::equal;
    if (a && b)
        return *a <=> *b;
    if (!a)
        return not_a;
    if (!b)
        return not_b;
    return std::strong_ordering::equal; // just for aviod warning...
}


template <typename T>
std::strong_ordering compare_null_first (std::optional <T> const & a, std::optional <T> const & b)
{
    return compare_null(a, b, std::strong_ordering::less, std::strong_ordering::greater);
}

template <typename T>
std::strong_ordering compare_null_last (std::optional <T> const & a, std::optional <T> const & b)
{
    return compare_null(a, b, std::strong_ordering::greater, std::strong_ordering::less);
}


#endif

