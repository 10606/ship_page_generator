#ifndef RESPONSE_SHIP_ARMAMENT_H
#define RESPONSE_SHIP_ARMAMENT_H

#include <string>
#include <string_view>
#include <vector>
#include <chrono>
#include <optional>
#include <map>
#include <deque>
#include <iostream>
#include "parse_query.h"
#include "date_to_str.h"
#include "ship_requests.h"
#include "ship_armament.h"
#include "guns.h"
#include "ship_names.h"


struct rows_table_template
{
    struct column_t
    {
        std::string_view begin = "\n<td>\n";
        std::string_view new_column = "\n</td>\n<td>\n";
        std::string_view new_line = "<br>\n";
        std::string_view end = "\n</td>\n";
    } column = column_t();
    
    std::string_view one_delimeter = "\n</tr>\n<tr>\n";
    std::string_view group_delimeter = "\n</tr>\n<tr>\n";

    struct rowspan_t
    {
        // <begin> number <middle> text <end>
        std::string_view begin = "<td rowspan=";
        std::string_view middle = ">"; 
        std::string_view end = "</td>";
    } rowspan = rowspan_t();
};


template <typename responser>
struct ships_responser
{
    template <typename ... T>
    ships_responser 
    (
        rows_table_template _table,
        ship_requests * _database,
        T && ... args
    ) :
        table(_table),
        database(_database),
        resp(_database, std::forward <T> (args) ...)
    {}
    
    std::vector <std::string> response 
    (
        std::vector <std::pair <int, std::chrono::year_month_day> > ship_year,
        std::vector <bool> modernization
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

private:
    rows_table_template table;
    ship_requests * database;
    responser resp;
};


struct table_template
{
    std::string_view begin = "<table border=1>\n<tr>\n";
    std::string_view new_line = "<br>\n";
    std::string_view new_row = "\n</tr>\n<tr>\n";
    std::string_view end = "\n</tr>\n</table>\n";
};


struct ship_armament
{
    ship_armament (table_template _table, ship_requests * _database) :
        table(_table),
        names(header_column(), _database),
        guns(rows_table_template(), _database, table.new_line)
    {}

    bool check (std::string_view uri)
    {
        return uri == "/ship/armament";
    }

    std::string response (std::string_view query)
    {
        std::vector <std::pair <int, std::chrono::year_month_day> > ship_year =
            parse_query__ship_year(query);
        
        try
        {
            auto [header, modernizations] = names.response(ship_year);
            std::string answer = std::string(table.begin);
            
            answer += header + std::string(table.new_row);
            try
            {
                std::vector <std::string> table_guns = guns.response(ship_year, modernizations);
                for (auto const & s : table_guns)
                    answer += s;
            }
            catch (...)
            {}
            
            answer += std::string(table.end);
            return answer;
        }
        catch (...)
        {
            return "";
        }
    }

private:
    table_template table;
    ship_names names;
    ships_responser <ship_guns> guns;
};


#endif

