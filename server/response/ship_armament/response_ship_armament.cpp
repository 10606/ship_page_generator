#include "response_ship_armament.h"

#include <future>


template <typename responser>
std::vector <std::pair <uint32_t, std::string_view> >
ships_responser <responser> ::gun_classes
(
    std::vector <std::vector <typename responser::response_t> > & values,
    std::optional <key_t> min
) const
{
    std::vector <std::pair <uint32_t, std::string_view> > gun_class = {{0, table.column.begin}};
    std::vector <size_t> positions(values.size(), 0);

    while (min)
    {
        bool have_one_delimeter = 0;
        bool have_group_delimeter = 0;
        key_t expect = *min;
        min.reset();
        
        for (size_t i = 0; i != values.size(); ++i)
        {
            for (size_t j = positions[i]; j != values[i].size(); )
            {
                key_t cur{values[i][j].group, values[i][j].compare};
                if (cur != expect)
                {
                    have_one_delimeter   |= (cur.first == expect.first);
                    have_group_delimeter |= (cur.first != expect.first);
                    positions[i] = j;
                    if (!min || *min > cur)
                        min = std::move(cur);
                    break;
                }
                
                gun_class.back().second = values[i][j].group_name;
                
                // update position if we at end
                if (++j == values[i].size())
                    positions[i] = values[i].size();
            }
        }
        
        // end of row
        gun_class.back().first++;
        if (!have_one_delimeter && have_group_delimeter)
            gun_class.emplace_back();
    }

    return gun_class;
}


template <typename responser>
void
ships_responser <responser> ::response 
(
    simple_string & answer,
    std::vector <std::pair <int, std::chrono::year_month_day> > const & ship_year,
    std::vector <uint8_t> const & modernization
) const
{
    answer.append(table.group_delimeter);
    std::vector <std::vector <response_t> > values;
    values.reserve(ship_year.size());
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

    
    std::vector <std::pair <uint32_t, std::string_view> > gun_class = gun_classes(values, min);
    std::vector <size_t> positions(ship_year.size(), 0);
    // main part of table
    uint32_t class_sum = 0, class_pos = 0;
    uint32_t rows_cnt = 0;
    while (min)
    {
        if (class_sum == rows_cnt)
        {
            // new gun_class
            answer.append(table.rowspan.begin);
            add_value(answer, gun_class[class_pos].first);
            answer.append(table.rowspan.middle)
                  .append(std::move(gun_class[class_pos].second))
                  .append(table.rowspan.end)
                  .append(table.column.begin);
            class_sum += gun_class[class_pos].first;
            class_pos++;
        }
        else
            answer.append(table.column.begin);
        rows_cnt++;
        
        bool have_one_delimeter = 0;
        bool have_group_delimeter = 0;
        key_t expect = *min;
        min.reset();
        
        for (size_t i = 0; i != ship_year.size(); ++i)
        {
            if (i != 0)
                answer.append(table.column.new_column);
            
            for (size_t j = positions[i]; j != values[i].size(); )
            {
                key_t cur{std::move(values[i][j].group), std::move(values[i][j].compare)};
                if (cur != expect)
                {
                    have_one_delimeter   |= (cur.first == expect.first);
                    have_group_delimeter |= (cur.first != expect.first);
                    positions[i] = j;
                    if (!min || *min > cur)
                        min = std::move(cur);
                    break;
                }
                
                if (j != positions[i]) // not first line
                    answer.append(table.column.new_line);
                answer.append(std::move(values[i][j].data_begin))
                      .append(std::move(values[i][j].data_end));
                
                // update position if we at end
                if (++j == values[i].size()) 
                    positions[i] = values[i].size();
            }
        }
        
        // end of row
        answer.append(table.column.end);
        if (have_one_delimeter)
            answer.append(table.one_delimeter);
        else if (have_group_delimeter)
            answer.append(table.group_delimeter);
    }
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

std::vector <std::pair <int, std::chrono::year_month_day> > ship_armament::parse_query__ship_year (std::string_view query)
{
    std::vector <std::pair <int, std::chrono::year_month_day> > answer;
    
    status_sy_t status = {status_sy_t::none, 0};
    std::optional <uint32_t> ship_id;
    std::optional <std::array <uint32_t, 3> > date;
    
    static std::string_view ship_id_str = "ship=";
    static std::string_view date_str = "date=";
 
    auto append_if_need = [this, &answer, &ship_id, &date] () -> void
    {
        if (!ship_id)
            return;
        
        if (!date)
        {
            std::unordered_map <int, std::chrono::year_month_day> ::iterator it =
                default_date.find(*ship_id);
            if (it != default_date.end())
                answer.emplace_back(*ship_id, it->second);
            return;
        }

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
            status.status = status_sy_t::none;
            continue;
        }
        
        switch (status.status)
        {
        case status_sy_t::none:
            if (c == ship_id_str[0]) 
            {
                append_if_need();
                status.status = status_sy_t::ship_id_str;
                status.pos = 1;
            }
            else if (c == date_str[0])
            {
                if (date)
                    append_if_need();
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


void ship_armament::response (simple_string & answer, std::string_view query, piece_t title)
{
    static const constexpr std::string_view title_text = "сравнение японских корабликов";
    answer.rewrite(title.position, title_text.substr(0, std::min(title_text.size(), title.size)));

    try
    {
        std::vector <std::pair <int, std::chrono::year_month_day> > ship_year =
            parse_query__ship_year(query);
    
        auto [header, modernizations] = names.response(ship_year);
        answer.append(table.begin);
        answer.append(std::move(header));
        
        add_armament(answer, general,       ship_year, modernizations);
        add_armament(answer, guns,          ship_year, modernizations);
        add_armament(answer, torpedo_tubes, ship_year, modernizations);
        add_armament(answer, throwers,      ship_year, modernizations);
        add_armament(answer, searchers,     ship_year, modernizations);
        add_armament(answer, catapult,      ship_year, modernizations);
        add_armament(answer, aircraft,      ship_year, modernizations);
        
        answer.append(table.end);
    }
    catch (...)
    {}
}

