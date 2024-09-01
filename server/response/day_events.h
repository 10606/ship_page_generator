#ifndef DAY_EVENTS_H
#define DAY_EVENTS_H

#include "ship_names_list.h"
#include "parse_query.h"


struct day_events
{
    day_events (ship_requests *, ship_names_list const & _ship_names) :
        ship_names(_ship_names)
    {
        std::vector <ship_names_list::ship_info_t> const & names = ship_names.names();
        for (size_t i = 0; i != names.size(); ++i)
        {
            if (names[i].commissioned)
                commissioned_by_day[index_of_day(*names[i].commissioned)].push_back(i);
            if (names[i].sunk_date)
                sunk_by_day[index_of_day(*names[i].sunk_date)].push_back(i);
        }
    }

    void response (simple_string & answer, std::string_view query, piece_t title)
    {
        static const constexpr std::string_view title_text = "судьба японских корабликов";
        answer.rewrite(title.position, title_text.substr(0, std::min(title_text.size(), title.size)));
        
        std::vector <ship_names_list::ship_info_t> const & names = ship_names.names();
        std::optional <std::chrono::year_month_day> date_from_query = parse(query);
        std::chrono::year_month_day date_for
            (date_from_query? *date_from_query : std::chrono::year_month_day(std::chrono::floor <std::chrono::days> (std::chrono::system_clock::now())));
        
        uint16_t today_id = index_of_day(date_for);
        unsigned day   = static_cast <unsigned> (date_for.day());
        unsigned month = static_cast <unsigned> (date_for.month());
        char date_str[5] = {static_cast <char> ('0' + day / 10),
                            static_cast <char> ('0' + day % 10),
                            '.',
                            static_cast <char> ('0' + month / 10),
                            static_cast <char> ('0' + month % 10)};
        answer.append("события ",
                      date_str,
                      "<br>");
        {
            if (!commissioned_by_day[today_id].empty())
                answer.append("<br><h2>вошли в строй:</h2>");
            ship_names_list::add_ship_t add_ship(names, answer);
            for (size_t index : commissioned_by_day[today_id])
                add_ship(index);
        }
        {
            if (!sunk_by_day[today_id].empty())
                answer.append("<br><h2>потоплены:</h2>");
            ship_names_list::add_ship_t add_ship(names, answer);
            for (size_t index : sunk_by_day[today_id])
                add_ship(index);
        }
    }
    
private:
    struct sum_month_table_t
    {
        consteval sum_month_table_t ()
        {
            value[0] = 0;
            value[1] = 0;
            for (size_t i = 2; i != value.size(); ++i)
                value[i] = value[i - 1] + days_in_year_table[i - 2];
        }
        
        static const constexpr std::array <uint16_t, 12> days_in_year_table =
            {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        std::array <uint16_t, 13> value; // indexed from 1
    };
    
    uint16_t index_of_day (std::chrono::year_month_day date)
    {
        static const constexpr sum_month_table_t sum_month_table;
        unsigned month = static_cast <unsigned> (date.month());
        unsigned day   = static_cast <unsigned> (date.day());
        return sum_month_table.value[month] + day - 1;
    }
    
    std::optional <std::chrono::year_month_day> parse (std::string_view query)
    {
        static const constexpr std::string_view etalon = "date=";
        if (std::mismatch(query.begin(), query.end(), etalon.begin(), etalon.end()).second != etalon.end())
            return std::nullopt;
            
        std::array <uint16_t, 2> parsed = {0, 0};
        size_t id = 0;
        size_t digits = 0;
        for (size_t i = etalon.size(); i != query.size(); ++i)
        {
            if (query[i] == '.')
            {
                if (digits == 0)
                    return std::nullopt;
                id++;
                digits = 0;
                if (id >= 2)
                    return std::nullopt;
                continue;
            }
            if (!std::isdigit(query[i]))
                return std::nullopt;
            parsed[id] = parsed[id] * 10 + (query[i] - '0');
            digits++;
            if (digits > 2)
                return std::nullopt;
        }
        if (id != 1)
            return std::nullopt;
        std::chrono::year_month_day answer(std::chrono::year(1944), std::chrono::month(parsed[1]), std::chrono::day(parsed[0]));
        if (answer.ok())
            return answer;
        else
            return std::nullopt;
    };
    
    ship_names_list const & ship_names;
    std::array <std::vector <size_t>, 366> commissioned_by_day;
    std::array <std::vector <size_t>, 366> sunk_by_day;
};

#endif

