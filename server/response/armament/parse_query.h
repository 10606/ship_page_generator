#ifndef PARSE_QUERY_H
#define PARSE_QUERY_H

#include <string_view>
#include <vector>

struct group
{
    std::string_view key;
    std::vector <std::string_view> values;
};

std::vector <group> parse_query (std::string_view query);

#endif

