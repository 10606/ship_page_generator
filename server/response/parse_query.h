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


struct id_or_group_t
{
    enum type_t
    {
        id,
        group,
    };

    id_or_group_t (type_t _type, int _value) :
        type(_type),
        value(_value)
    {}
    
    type_t type;
    int value;
};

std::vector <id_or_group_t> parse_query__id (std::string_view query);

struct piece_t
{
    size_t position;
    size_t size;
};

std::string percent_dec (std::string_view request, bool need_escape = 0);

#endif

