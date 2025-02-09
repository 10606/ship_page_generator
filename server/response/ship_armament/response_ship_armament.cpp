#include "response_ship_armament.h"

#include "allocator.h"


template <typename responser>
void
ships_responser <responser> ::response 
(
    simple_string & answer,
    std::span <std::pair <int, std::chrono::year_month_day> const> ship_year,
    std::vector <uint8_t> const & modernization
) const
{
    answer.append(table.group_delimeter);
    std::vector <std::vector <response_t, allocator_for_temp <response_t> > > values;
    values.reserve(ship_year.size());
    
    std::optional <key_t> min;
    std::string_view group_name;

    // extract
    for (size_t i = 0; i != ship_year.size(); ++i)
    {
        if (modernization[i]) [[unlikely]]
        {
            values.emplace_back();
            continue;
        }
        
        values.emplace_back(resp.response(ship_year[i].first, ship_year[i].second));
        if (!values.back().empty()) [[likely]]
        {
            response_t const & tmp = values.back().front();
            key_t cur{tmp.group, tmp.compare};
            if (!min || *min > cur)
            {
                min = cur;
                group_name = tmp.group_name;
            }
        }
    }

    
    // main part of table
    std::vector <size_t> positions(ship_year.size(), 0);
    uint32_t rows_in_group = 1;
    bool new_group = 1;
    
    // for lazy set height rowspan
    static const constexpr size_t rows_cnt_max_width = 16;
    static const std::string height_placeholder(rows_cnt_max_width, ' ');
    size_t height_pos;
    
    while (min)
    {
        if (new_group)
        {
            // new gun_class
            height_pos = answer.size() + table.rowspan.begin.size();
            answer.append
            (
                table.rowspan.begin,
                height_placeholder,
                table.rowspan.middle,
                group_name,
                table.rowspan.end,
                table.column.begin
            );
            rows_in_group = 1;
        }
        else
            answer.append(table.column.begin);
        
        bool have_one_delimeter = 0;
        bool have_group_delimeter = 0;
        key_t expect = *min;
        min.reset();
        
        for (size_t i = 0; i != ship_year.size(); ++i)
        {
            if (i != 0) [[likely]]
                answer.append(table.column.new_column);
            
            for (size_t j = positions[i]; j != values[i].size(); )
            {
                key_t cur{values[i][j].group, values[i][j].compare};
                if (cur != expect)
                {
                    have_one_delimeter   |= (cur.group == expect.group);
                    have_group_delimeter |= (cur.group != expect.group);
                    positions[i] = j;
                    if (!min || *min > cur)
                    {
                        min = std::move(cur);
                        group_name = values[i][j].group_name;
                    }
                    break;
                }
                
                if (j != positions[i]) [[likely]] // not first line
                    answer.append(table.column.new_line);
                answer.append
                (
                    std::move(values[i][j].data_begin),
                    std::move(values[i][j].data_end)
                );
                
                // update position if we at end
                if (++j == values[i].size()) 
                    positions[i] = values[i].size();
            }
        }
        
        // end of row
        answer.append(table.column.end);
        new_group = 0;
        if (have_one_delimeter)
        {
            answer.append(table.one_delimeter);
            rows_in_group++;
        }
        else 
        {
            if (have_group_delimeter)
                answer.append(table.group_delimeter);
            new_group = 1;
            char rows_str[rows_cnt_max_width];
            std::to_chars_result res = std::to_chars(std::begin(rows_str), std::end(rows_str), rows_in_group);
            answer.rewrite(height_pos, std::string_view(rows_str, res.ptr));
        }
    }
}


enum status_sy_t
{
    none,
    ship_id_value,
    date_value,
    error
};

std::vector <std::pair <int, std::chrono::year_month_day> > ship_armament::parse_query__ship_year (std::string_view query)
{
    std::vector <std::pair <int, std::chrono::year_month_day> > answer;
    
    status_sy_t status = status_sy_t::none;
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
            if (it != default_date.end()) [[likely]]
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
        
        if (!query_date.ok()) [[unlikely]]
        {
            date.reset();
            return;
        }
        
        answer.emplace_back(*ship_id, query_date);
        date.reset();
    };
    
    for (size_t i = 0; i != query.size(); )
    {
        char c = query[i];
        if (c == '&') [[unlikely]]
        {
            status = status_sy_t::none;
            i++;
            continue;
        }
        
        auto check_etalon = [&i, &status, &query] (std::string_view etalon, status_sy_t next) -> bool
        {
            size_t pos = 0;
            status = status_sy_t::error;
            while (pos != etalon.size() && i != query.size())
            {
                char c = query[i];
                if (c != etalon[pos]) [[unlikely]]
                    break;
                pos++;
                i++;
            }
            if (pos == etalon.size()) [[likely]]
            {
                status = next;
                return 1;
            }
            else
                return 0;
        };
        
        switch (status)
        {
        case status_sy_t::none:
            if (c == ship_id_str[0])
            {
                append_if_need();
                if (check_etalon(ship_id_str, status_sy_t::ship_id_value))
                    ship_id.reset();
            }
            else if (c == date_str[0])
            {
                if (date)
                    append_if_need();
                if (check_etalon(date_str, status_sy_t::date_value))
                    date.reset();
            }
            else
                status = status_sy_t::error;
            break;
        case status_sy_t::ship_id_value:
            if (std::isdigit(c)) [[likely]]
                ship_id = 0;
            while (i != query.size())
            {
                c = query[i];
                i++;
                if (std::isdigit(c)) [[likely]]
                {
                    *ship_id = *ship_id * 10 + c - '0';
                }
                else if (c == '&') [[likely]]
                {
                    status = status_sy_t::none;
                    break;
                }
                else
                {
                    status = status_sy_t::error;
                    break;
                }
            }
            break;
        case status_sy_t::date_value:
            if (std::isdigit(c)) [[likely]]
                date = {0, 0, 0};
            for (size_t pos = 0; i != query.size(); )
            {
                c = query[i];
                i++;
                if (std::isdigit(c)) [[likely]]
                {
                    (*date)[pos] = (*date)[pos] * 10 + c - '0';
                }
                else if (c == '.') [[likely]]
                {
                    if (++pos == date->size()) [[unlikely]]
                        status = status_sy_t::error;
                }
                else if (c == '&') [[likely]]
                {
                    status = status_sy_t::none;
                    break;
                }
                else
                {
                    status = status_sy_t::error;
                    break;
                }
            }
            break;
        case status_sy_t::error:
            i++;
            break;
        }
    }

    append_if_need();

    return answer;
}


void ship_armament::response (simple_string & answer, std::vector <std::pair <int, std::chrono::year_month_day> > const & ship_year, bool add_checkbox)
{
    size_t parts = ship_year.size() / 7 + 1;
    
    size_t begin = 0;
    for (size_t i = 0; i != parts; ++i)
    {
        if (begin != 0)
            answer.append(table.new_line);
        size_t end = ship_year.size() * (i + 1) / parts;
        std::span <std::pair <int, std::chrono::year_month_day> const> cur_ship_year(ship_year.begin() + begin, ship_year.begin() + end);
        begin = end;
        
        answer.append(table.begin);
        std::vector <uint8_t> modernizations = names.response(answer, cur_ship_year, add_checkbox);
        
        add_armament(answer, general,       cur_ship_year, modernizations);
        add_armament(answer, guns,          cur_ship_year, modernizations);
        add_armament(answer, torpedo_tubes, cur_ship_year, modernizations);
        add_armament(answer, throwers,      cur_ship_year, modernizations);
        add_armament(answer, searchers,     cur_ship_year, modernizations);
        add_armament(answer, catapult,      cur_ship_year, modernizations);
        add_armament(answer, aircraft,      cur_ship_year, modernizations);
        add_armament(answer, propulsion,    cur_ship_year, modernizations);
            
        answer.append(table.end);
    }
}

void ship_armament::response (simple_string & answer, std::string_view query, piece_t title, bool add_checkbox)
{
    static const constexpr std::string_view title_text = "сравнение японских корабликов";
    answer.rewrite(title.position, title_text.substr(0, std::min(title_text.size(), title.size)));

    try
    {
        std::vector <std::pair <int, std::chrono::year_month_day> > ship_year =
            parse_query__ship_year(query);
        response(answer, ship_year, add_checkbox);
    }
    catch (...)
    {}
}

