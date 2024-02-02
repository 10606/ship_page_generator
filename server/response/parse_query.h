#ifndef PARSE_QUERY_H
#define PARSE_QUERY_H

#include <string_view>
#include <vector>
#include <chrono>

struct group
{
    group
    (
        std::string_view _key,
        std::vector <std::string_view> _values
    ) :
        key(_key),
        values(std::move(_values))
    {}

    std::string_view key;
    std::vector <std::string_view> values;
};

std::vector <group> parse_query (std::string_view query);

std::vector <int> parse_query__id (std::string_view query);

struct piece_t
{
    size_t position;
    size_t size;
};

std::string percent_dec (std::string_view request, bool need_escape = 0);

#endif

