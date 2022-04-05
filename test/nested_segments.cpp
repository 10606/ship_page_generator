#include "nested_segments.h"

#include <gtest/gtest.h>

constexpr std::chrono::year_month_day ymd (int year, unsigned month, unsigned day)
{
    return std::chrono::year_month_day(std::chrono::year(year), std::chrono::month(month), std::chrono::day(day));
}

TEST(segment, simple)
{
    std::vector <segment> segments =
    {
        {ymd(41, 8, 2), ymd(43, 1, 3), 0},
        {ymd(42, 2, 2), ymd(44, 3, 3), 1},
        {ymd(43, 5, 2), ymd(44, 7, 3), 2},
        {ymd(44, 2, 2), ymd(45, 1, 3), 3},
        {ymd(44, 6, 2), ymd(45, 6, 3), 4},
        {ymd(44, 6, 2), ymd(45, 6, 3), 5}, // same as 4
        
        {ymd(42, 1, 8), ymd(42, 9, 3), 6}, // 0
        {ymd(42, 6, 8), ymd(42, 9, 3), 7}, // 0 1
    };

    std::vector <std::vector <size_t> > nested =
        nested_segments(segments);
    
    std::vector <std::vector <size_t> > etalon =
        {{6}, {7}, {}, {}, {}, {}, {}, {}};
    
    EXPECT_EQ(nested, etalon);
}

TEST(segment, null)
{
    std::vector <segment> segments =
    {
        {std::nullopt,  std::nullopt,  0},
        {ymd(42, 4, 3), std::nullopt,  1}, // 0
        {std::nullopt,  ymd(44, 5, 3), 2}, // 0
        {ymd(42, 9, 5), ymd(43, 8, 4), 3}  // 0 1
    };

    std::vector <std::vector <size_t> > nested =
        nested_segments(segments);
    
    std::vector <std::vector <size_t> > etalon =
        {{2, 1}, {3}, {}, {}};
    
    EXPECT_EQ(nested, etalon);
}

