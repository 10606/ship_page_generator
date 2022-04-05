#include "parse_query.h"

#include <string_view>
#include <cctype>
#include <vector>


enum class status_t
{
    key,
    values,
    skip
};

bool check_symbol (char c)
{
    return  std::isalpha(c) || 
            std::isdigit(c) ||
            c == '-' ||
            c == '_';
}

// a=1,z,3&b=7,8,q&c=1
std::vector <group> parse_query (std::string_view query)
{
    std::vector <group> answer;
    size_t start_pos = 0;
    status_t status = status_t::key;
    
    for (size_t i = 0; i != query.size(); ++i)
    {
        switch (status)
        {
        case status_t::key:
            if (query[i] == '=' || query[i] == '&')
            {
                answer.emplace_back(query.substr(start_pos, i - start_pos), std::vector <std::string_view> ());
                start_pos = i + 1;
                status = query[i] == '&' ? status_t::key : status_t::values;
            }
            else if (!check_symbol(query[i]))
                status = status_t::skip;
            break;
        case status_t::values:
            if (query[i] == ',' || query[i] == '&')
            {
                answer.back().values.push_back(query.substr(start_pos, i - start_pos));
                start_pos = i + 1;
                if (query[i] == '&')
                    status = status_t::key;
            }
            else if (!check_symbol(query[i]))
                status = status_t::skip;
            break;
        case status_t::skip:
            if (query[i] == '&')
            {
                start_pos = i + 1;
                status = status_t::key;
            }
            break;
        }
    }
    
    switch (status)
    {
    case status_t::key:
        answer.emplace_back(query.substr(start_pos), std::vector <std::string_view> ());
        break;
    case status_t::values:
        answer.back().values.push_back(query.substr(start_pos));
        break;
    default:
        break;
    }
    
    return answer;
}


