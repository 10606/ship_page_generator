#ifndef SHIP_ARMAMENT_UTILS_H
#define SHIP_ARMAMENT_UTILS_H

#include <optional>
#include <chrono>

inline bool between 
(
    std::optional <std::chrono::year_month_day> date_from,
    std::chrono::year_month_day date,
    std::optional <std::chrono::year_month_day> date_to
)
{
    return  (!date_from || *date_from <= date) &&
            (!date_to   || date < *date_to);
}

#endif

