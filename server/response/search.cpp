#include "search.h"

#include <set>
#include <iostream>

#include "dist_livenshtein.h"


void search::response (simple_string & answer, std::string_view request_percent_enc, piece_t title)
{
    static const constexpr std::string_view title_text = "поиск японских корабликов";
    answer.rewrite(title.position, title_text.substr(0, std::min(title_text.size(), title.size)));

    std::vector <ship_names_list::ship_info_t> const & names = ship_names.names();
    if (request_percent_enc.starts_with(search_keyword))
        request_percent_enc = request_percent_enc.substr(search_keyword.size());
    std::string request = percent_dec(request_percent_enc);

    if (request.empty())
        return;
    ship_names_list::add_ship_t add_ship(names, answer);
    if (request.size() <= 1)
        for (uint32_t pos : by_1_chars[static_cast <uint8_t> (request[0])])
            add_ship(pos);
    else if (request.size() <= 3)
    {
        auto [answer_list, check_3th_offset] = calc_index_3(request);
        if (!answer_list)
            return;

        for (position_t pos : *answer_list)
        {
            if (request.size() >= 3)
            {
                if ((names[pos.index].name_ru.size() <= pos.offset + check_3th_offset ||
                     names[pos.index].name_ru[pos.offset + check_3th_offset] != request[check_3th_offset]) &&
                    (names[pos.index].name_en.size() <= pos.offset + check_3th_offset ||
                     names[pos.index].name_en[pos.offset + check_3th_offset] != request[check_3th_offset]))
                    continue;
            } else if (check_3th_offset != 2)
                continue;
            add_ship(pos.index);
        }
    }
    else
    {
        uint32_t index = calc_index_4(request);
        auto it = by_4_chars.find(index);
        if (it != by_4_chars.end())
        {
            for (position_t const & pos : it->second)
            {
                bool match = 0;
                
                if (names[pos.index].name_ru.size() >= pos.offset + request.size())
                {
                    std::string_view name_ru = 
                        std::string_view(names[pos.index].name_ru.data() + pos.offset, request.size());
                    match |= (name_ru == request);
                }
                if (names[pos.index].name_en.size() >= pos.offset + request.size())
                {
                    std::string_view name_en = 
                        std::string_view(names[pos.index].name_en.data() + pos.offset, request.size());
                    match |= (name_en == request);
                }

                if (match)
                    add_ship(pos.index);
            }
        }
    }

    if (add_ship.empty())
    {
        size_t min_dist = std::numeric_limits <size_t> ::max();
        std::set <std::pair <size_t, size_t> > matched; // cost, index
        for (size_t i = 0; i != names.size(); ++i)
        {
            static const constexpr size_t max_diff_cost = 15;
            size_t threshold = 30;
            if (request.size() <= 2)
                threshold = 10;
            else if (request.size() == 3)
                threshold = 15;
            else if (request.size() == 4)
                threshold = 20;
            
            size_t cur = dist_livenshtein(request, names[i].name_en);
            if (cur > threshold)
                continue;
            if (cur < min_dist)
            {
                min_dist = cur;
                matched.insert({cur, i});
                while (!matched.empty())
                {
                    std::set <std::pair <size_t, size_t> > ::iterator it = matched.end();
                    it--;
                    if (it->first <= min_dist + max_diff_cost)
                        break;
                    matched.erase(it);
                }
            }
            else if (cur <= min_dist + max_diff_cost)
                matched.insert({cur, i});
        }
       
        std::vector <size_t> selected_ships;
        selected_ships.reserve(matched.size());
        for (auto ship : matched)
            selected_ships.push_back(ship.second);
        std::sort(selected_ships.begin(), selected_ships.end());
        
        for (size_t ship_id : selected_ships)
            add_ship(ship_id);
    }
    
    add_ship.close();
}

void search::add (std::string_view name, size_t name_index)
{
    for (size_t i = 0; i != name.size(); ++i)
    {
        std::string_view value = name.substr(i);
        position_t pos = {static_cast <uint32_t> (name_index), static_cast <uint32_t> (i)};
        uint8_t cur = value[0];
        if (by_1_chars[cur].empty() || by_1_chars[cur].back() != pos.index)
            by_1_chars[cur].emplace_back(pos.index);
        if (value.size() >= 2)
        {
            if (is_mb2_begin(value[0]) && is_mb(value[1]))
                by_2_chars_mb2[cur][static_cast <uint8_t> (value[1]) & 0b00111111].emplace_back(pos);
            else if (is_ascii(value[0]))
            {
                std::pair <uint16_t, size_t> index = calc_index_3_ascii_(value);
                by_2_chars_ascii[index.first].emplace_back(pos);
            }
        }
        if (value.size() >= 4)
        {
            // see 2, 3, 4 symbols
            for (size_t i = 2; i != 5; ++i)
            {
                uint32_t index = calc_index_4(value, i);
                by_4_chars[index].emplace_back(pos);
            }
        }
    }
}

uint32_t search::calc_index_4 (std::string_view request, size_t use_symbols)
{
    uint32_t index = 0;
    for (size_t i = 0, pos = 0; i != use_symbols && pos < request.size(); ++i)
    {
        index = index << 8;
        char cur = request[pos];
        size_t need_check = 0;
        if (is_ascii(cur))
            need_check = 0;
        else if (is_mb2_begin(cur))
            need_check = 1;
        else if (is_mb3_begin(cur))
            need_check = 2;
        else if (is_mb4_begin(cur))
            need_check = 3;
        pos++;
        if (pos + need_check > request.size())
        {
            i--;
            // std::cerr << "wrong utf-8 sequence (not full): \"" << request << "\"  pos: " << i << std::endl;
            continue;
        }
        for (size_t i = 0; i != need_check; ++i, pos++)
        {
            if (!is_mb(request[pos]))
            {
                i--;
                // std::cerr << "wrong utf-8 sequence (expected 0b01xxxxxx): \"" << request << "\"  pos: " << i << std::endl;
                continue;
            }
        }
        index += static_cast <uint8_t> (request[pos - 1]);
    }
    return index;
}

std::string search::get_search_parameter (std::string_view request)
{
    size_t begin = request.find(search_keyword);
    if (begin == std::string_view::npos)
        return std::string();
    if (begin != 0 && request[begin - 1] != '&')
        return std::string();

    size_t end = request.find('&', begin);

    begin += search_keyword.size();
    request = request.substr(begin, (end == std::string_view::npos)? end : end - begin);
    return percent_dec(request, 1);
}

