#include "search.h"

#include <set>
#include <iostream>

struct add_ship_t
{
    add_ship_t (search const & _search_data, simple_string & _answer) :
        search_data(_search_data),
        answer(_answer),
        class_id(),
        old_pos()
    {}

    void operator () (uint32_t pos)
    {
        if (old_pos == pos)
            return;
        old_pos = pos;
        if (search_data.names[pos].class_id != class_id)
        {
            if (class_id)
                answer.append("</table>");
            answer.append("<br><table>");
            class_id = search_data.names[pos].class_id;
        }
        answer.append(search_data.names[pos].answer);
    }

    void close ()
    {
        if (class_id)
            answer.append("</table>");
        class_id.reset();
    }
    
    ~add_ship_t ()
    {
        try
        {
            close();
        }
        catch (...)
        {}
    }
    
private:
    search const & search_data;
    simple_string & answer;
    std::optional <int> class_id;
    std::optional <uint32_t> old_pos;
};


std::string search::percent_dec (std::string_view request_percent_enc, bool need_escape)
{
    static const constexpr std::string_view quot = "&quot;";

    std::string answer;
    answer.reserve(request_percent_enc.size());
    
    for (size_t i = 0; i != request_percent_enc.size(); ++i)
    {
        if (request_percent_enc[i] != '%' ||
            i + 2 >= request_percent_enc.size())
        {
            if (need_escape && request_percent_enc[i] == '"')
                answer.append(quot);
            else
                answer.push_back(request_percent_enc[i]);
            continue;
        }
        if (!std::isxdigit(request_percent_enc[i + 1]) ||
            !std::isxdigit(request_percent_enc[i + 2]))
        {
            answer.push_back(request_percent_enc[i]);
            continue;
        }
        uint8_t cur = 0;
        cur = (cur << 4) + from_hex(request_percent_enc[i + 1]);
        cur = (cur << 4) + from_hex(request_percent_enc[i + 2]);
        if (need_escape && cur == '"')
            answer.append(quot);
        else
            answer.push_back(cur);
        i += 2;
    }
    return answer;
}

void search::response (simple_string & answer, std::string_view request_percent_enc, piece_t title)
{
    static const constexpr std::string_view title_text = "поиск японских корабликов";
    answer.rewrite(title.position, title_text.substr(0, std::min(title_text.size(), title.size)));

    if (request_percent_enc.starts_with(search_keyword))
        request_percent_enc = request_percent_enc.substr(search_keyword.size());
    std::string request = percent_dec(request_percent_enc);

    if (request.empty())
        return;
    add_ship_t add_ship(*this, answer);
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
                if (names[pos.index].name_ru.size() < pos.offset + request.size() &&
                    names[pos.index].name_en.size() < pos.offset + request.size())
                    continue;
                
                std::string_view name_ru = 
                    std::string_view(names[pos.index].name_ru.data() + pos.offset, request.size());
                std::string_view name_en = 
                    std::string_view(names[pos.index].name_en.data() + pos.offset, request.size());

                if (name_ru != request && name_en != request)
                    continue;
                add_ship(pos.index);
            }
        }
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
            uint32_t index = calc_index_4(value);
            by_4_chars[index].emplace_back(pos);
        }
    }
}

uint32_t search::calc_index_4 (std::string_view request)
{
    uint32_t index = 0;
    for (size_t i = 0, pos = 0; i != 4 && pos < request.size(); ++i)
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
            std::cerr << "wrong utf-8 sequence (not full): \"" << request << "\"  pos: " << i << std::endl;
            continue;
        }
        if (pos + need_check >= 4)
            break;
        for (size_t i = 0; i != need_check; ++i, pos++)
        {
            if (!is_mb(request[pos]))
            {
                i--;
                std::cerr << "wrong utf-8 sequence (expected 0b01xxxxxx): \"" << request << "\"  pos: " << i << std::endl;
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

