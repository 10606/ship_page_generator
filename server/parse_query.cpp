#include "parse_query.h"

#include <optional>
#include <cctype>
#include <array>


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
                *answer = 0;
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




struct status_sy_t
{
    enum 
    {
        none,
        ship_id_str,
        ship_id_value,
        date_str,
        date_value,
        error
    } status;
    
    size_t pos;
};

std::vector <std::pair <int, std::chrono::year_month_day> > parse_query__ship_year (std::string_view query)
{
    std::vector <std::pair <int, std::chrono::year_month_day> > answer;
    
    status_sy_t status = {status_sy_t::none, 0};
    std::optional <uint32_t> ship_id;
    std::optional <std::array <uint32_t, 3> > date;
    
    static std::string_view ship_id_str = "ship=";
    static std::string_view date_str = "date=";
 
    auto append_if_need = [&answer, &status, &ship_id, &date] () -> void
    {
        status.status = status_sy_t::none;

        if (!date || !ship_id)
            return;

        if ((*date)[2] < 100)
            (*date)[2] += 1900;
        
        std::chrono::year_month_day query_date
        {
            std::chrono::year((*date)[2]),
            std::chrono::month((*date)[1]),
            std::chrono::day((*date)[0])
        };
        
        if (!query_date.ok())
        {
            date.reset();
            return;
        }
        
        answer.emplace_back(*ship_id, query_date);
        date.reset();
    };
    
    for (char c : query)
    {
        if (c == '&')
        {
            append_if_need();
            continue;
        }
        
        switch (status.status)
        {
        case status_sy_t::none:
            if (c == ship_id_str[0]) 
            {
                status.status = status_sy_t::ship_id_str;
                status.pos = 1;
            }
            else if (c == date_str[0])
            {
                status.status = status_sy_t::date_str;
                status.pos = 1;
            }
            else
                status.status = status_sy_t::error;
            break;
        case status_sy_t::ship_id_str:
            if (c == ship_id_str[status.pos])
            {
                if (++status.pos == ship_id_str.size())
                {
                    ship_id.reset();
                    status.status = status_sy_t::ship_id_value;
                    status.pos = 0;
                }
            }
            else
                status.status = status_sy_t::error;
            break;
        case status_sy_t::ship_id_value:
            if (std::isdigit(c))
            {
                if (!ship_id)
                    ship_id = 0;
                *ship_id = *ship_id * 10 + c - '0';
            }
            else
                status.status = status_sy_t::error;
            break;
        case status_sy_t::date_str:
            if (c == date_str[status.pos])
            {
                if (++status.pos == date_str.size())
                {
                    date.reset();
                    status.status = status_sy_t::date_value;
                    status.pos = 0;
                }
            }
            else
                status.status = status_sy_t::error;
            break;
        case status_sy_t::date_value:
            if (std::isdigit(c))
            {
                if (!date)
                    date = {0, 0, 0};
                (*date)[status.pos] = (*date)[status.pos] * 10 + c - '0';
            }
            else if (c == '.')
            {
                if (++status.pos == date->size())
                    status.status = status_sy_t::error;
            }
            else
                status.status = status_sy_t::error;
            break;
        case status_sy_t::error:
            break;
        }
    }

    append_if_need();

    return answer;
}

