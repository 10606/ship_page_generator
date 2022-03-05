#ifndef PARSE_QUERY_H
#define PARSE_QUERY_H

#include <string_view>
#include <vector>
#include <chrono>

std::vector <int> parse_query__id (std::string_view query);
std::vector <std::pair <int, std::chrono::year_month_day> > parse_query__ship_year (std::string_view query);

#endif

