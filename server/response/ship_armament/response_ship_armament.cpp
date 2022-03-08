#include "response_ship_armament.h"


template <typename responser>
std::vector <std::string> ships_responser <responser> ::response 
(
    std::vector <std::pair <int, std::chrono::year_month_day> > ship_year,
    std::vector <uint8_t> modernization
)
{
    using response_t = typename responser::response_t;
    std::vector <std::vector <response_t> > values;
    values.reserve(ship_year.size());

    using key_t = std::pair <decltype(std::declval <response_t>().group),
                             decltype(std::declval <response_t>().compare)>;
    std::optional <key_t> min;

    // extract
    for (size_t i = 0; i != ship_year.size(); ++i)
    {
        if (modernization[i])
        {
            values.emplace_back();
            continue;
        }
        
        values.emplace_back(resp.response(ship_year[i].first, ship_year[i].second));
        if (!values.back().empty())
        {
            response_t const & tmp = values.back().front();
            key_t cur{tmp.group, tmp.compare};
            if (!min || *min > cur)
                min = cur;
        }
    }

    
    std::vector <std::string> rows;
    std::vector <std::pair <size_t, std::string> > gun_class = {{0, std::string(table.column.begin)}};
    std::vector <size_t> positions(ship_year.size());
    // main part of table
    while (min)
    {
        rows.emplace_back(table.column.begin);
        bool have_one_delimeter = 0;
        bool have_group_delimeter = 0;
        key_t expect = *min;
        min.reset();
        
        for (size_t i = 0; i != ship_year.size(); ++i)
        {
            if (i != 0)
                rows.back() += table.column.new_column;
            
            for (size_t j = positions[i]; j != values[i].size(); )
            {
                response_t const & tmp = values[i][j];
                key_t cur{tmp.group, tmp.compare};
                if (cur != expect)
                {
                    if (tmp.group == expect.first)
                        have_one_delimeter = 1;
                    else
                        have_group_delimeter = 1;
                    positions[i] = j;
                    if (!min || *min > cur)
                        min = cur;
                    break;
                }
                
                gun_class.back().second = tmp.group_name;
                if (j != positions[i]) // not first line
                    rows.back() += table.column.new_line;
                rows.back() += values[i][j].data;
                
                // update position if we at end
                if (++j == values[i].size())
                    positions[i] = values[i].size();
            }
        }
        
        // end of row
        rows.back() += table.column.end;
        if (have_one_delimeter)
        {
            gun_class.back().first++;
            rows.back() += table.one_delimeter;
        }
        else if (have_group_delimeter)
        {
            gun_class.back().first++;
            gun_class.emplace_back();
            rows.back() += table.group_delimeter;
        }
        else
        {
            gun_class.back().first++;
        }
    }
    
    if (rows.empty())
        return rows;
    
    // gun classes
    for (size_t i = 0, pos = 0; i != gun_class.size(); ++i)
    {
        std::string rowspan = std::string(table.rowspan.begin) + 
                              std::to_string(gun_class[i].first) + 
                              std::string(table.rowspan.middle) + 
                              gun_class[i].second + 
                              std::string(table.rowspan.end);
        rows[pos] = rowspan + rows[pos];
        pos += gun_class[i].first;
    }
    return rows;
}


std::string ship_armament::response (std::string_view query)
{
    std::vector <std::pair <int, std::chrono::year_month_day> > ship_year =
        parse_query__ship_year(query);
    
    try
    {
        auto [header, modernizations] = names.response(ship_year);
        std::string answer = std::string(table.begin);
        
        answer += header;
        
        answer += add_armament(guns, ship_year, modernizations, table.new_row);
        answer += add_armament(torpedo_tubes, ship_year, modernizations, table.new_row);
        answer += add_armament(throwers, ship_year, modernizations, table.new_row);
        answer += add_armament(searchers, ship_year, modernizations, table.new_row);
        answer += add_armament(catapult, ship_year, modernizations, table.new_row);
        answer += add_armament(aircraft, ship_year, modernizations, table.new_row);
        
        answer += std::string(table.end);
        return answer;
    }
    catch (...)
    {
        return "";
    }
}

