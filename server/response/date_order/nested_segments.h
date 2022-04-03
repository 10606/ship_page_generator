#ifndef NESTED_SEGMENTS_H
#define NESTED_SEGMENTS_H


#include <chrono>
#include <vector>
#include <optional>

struct segment
{
    std::chrono::year_month_day begin;
    std::chrono::year_month_day end;
    size_t index;
};

std::vector <std::vector <size_t> >
nested_segments 
(std::vector <segment> values);


#endif

