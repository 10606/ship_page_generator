#include "parse_query.h"

#include <string_view>
#include <optional>
#include <cctype>
#include <array>
#include <vector>


std::pair <bool, std::string_view::const_iterator> parse_string 
(
    std::string_view::const_iterator begin, 
    std::string_view::const_iterator end,
    std::string_view pattern
)
{
    std::pair <std::string_view::const_iterator,
               std::string_view::const_iterator> it = 
        std::mismatch(begin, end, pattern.begin(), pattern.end());

    return {it.second == pattern.end(), it.first};
}


std::pair <std::optional <uint32_t>, std::string_view::const_iterator> parse_number
(
    std::string_view::const_iterator begin, 
    std::string_view::const_iterator end
)
{
    std::optional <uint32_t> answer;
    
    if (begin == end)
        return {std::nullopt, end};
    
    for (; begin != end; begin++)
    {
        char c = *begin;
        if (std::isdigit(c))
        {
            if (!answer)
                answer = 0;
            *answer = *answer * 10 + c - '0';
        }
        else
            return {answer, begin};
    }
    
    return {answer, end};
}

std::vector <int> parse_query__id (std::string_view query)
{
    std::vector <int> answer;

    static std::string_view id_str = "id=";
    
    for (std::string_view::const_iterator it = query.begin(); it != query.end(); )
    {
        auto cur_end = [&it, query] () -> bool
        {
            return (it == query.end() || *it == '&');
        };
        
        auto skip = [&it, query, &cur_end] () -> void
        {
            while (!cur_end())
                ++it;
            if (it != query.end())
                ++it;
        };
    
        std::pair <bool, std::string_view::const_iterator> skipped = parse_string(it, query.end(), id_str);
        it = skipped.second;
        if (!skipped.first)
        {
            skip();
            continue;
        }
        
        std::pair <std::optional <uint32_t>, std::string_view::const_iterator> parsed_id = parse_number(it, query.end());
        it = parsed_id.second;
        if (!parsed_id.first || !cur_end())
        {
            skip();
            continue;
        }
        else
        {
            answer.emplace_back(*parsed_id.first);
            if (it != query.end())
                ++it;
        }
    }
    
    return answer;
}


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

