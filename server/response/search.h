#ifndef SEARCH_H
#define SEARCH_H

#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include "ship_requests.h"
#include "ship_info.h"
#include "simple_string.h"
#include "armament_links.h"

struct search
{
    search (ship_requests * database)
    {
        std::vector <ship_info_long> ship_names =
            database->ship_info.get_list("order by ((select get_ancestor_by_id (ship_list.class_id)), ship_list.class_id, \
                                                    ship_list.type_id, commissioned, ship_list.name_ru,  ship_list.id)");

        names.reserve(ship_names.size());
        for (ship_info_long const & ship_info : ship_names)
            names.emplace_back(ship_info);
        
        for (size_t i = 0; i != names.size(); ++i)
        {
            add(names[i].name_ru, i);
            add(names[i].name_en, i);
        }
    }
    
    void response (simple_string & answer, std::string_view request);
    
private:
    typedef ship_requests::ship_info_t::list ship_info_long;

    struct position_t
    {
        uint32_t index;
        uint32_t offset;
    };
    
    struct ship_info_t
    {
        ship_info_t (ship_info_long value) :
            class_id(value.class_id),
            name_en(value.ship_en.value_or("")),
            name_ru(value.ship_ru.value_or("")),
            answer()
        {
            answer.append("<tr><td>")
                  .append(armament_links::base("/ship?id=" + std::to_string(value.ship_id), value.ship_ru.value_or("--")))
                  .append(" ");
            if (value.class_ru || value.type_ru)
            {
                answer.append("</td><td>(")
                      .append(value.class_ru.value_or(""));
                if (value.type_ru)
                    answer.append(" типа ")
                          .append(*value.type_ru);
                answer.append(")");
            }
            answer.append("</td></tr>");
        }

        int class_id;
        std::string name_en;
        std::string name_ru;
        std::string answer;
    };
    
    std::vector <ship_info_t> names;
    std::array <std::vector <uint32_t>, 256> by_1_chars;
    std::unordered_map <uint8_t, std::array <std::vector <position_t>, (1lu << 6)> > by_2_chars_mb2; // multi_byte 2
    std::array <std::vector <position_t>, (1lu << (7 + 7))> by_2_chars_ascii; // ascii | acsii,  ascii | mb <n> here use second char
    std::unordered_map <uint32_t, std::vector <position_t> > by_4_chars; // last chars for multi byte
    
    void add (std::string_view value, size_t name_index);
    static std::string percent_dec (std::string_view request);
    
    static uint8_t from_hex (char c)
    {
        if (std::isdigit(c))
            return c - '0';
        if (std::islower(c))
            return c - 'a' + 10;
        return c - 'A' + 10;
    }

    template <template <typename, typename ...> typename T, typename ... U> 
    void add_ship (simple_string & answer, T <uint32_t, U ...> const & container)
    {
        std::optional <int> class_id;
        for (size_t pos : container)
        {
            if (names[pos].class_id != class_id)
            {
                if (class_id)
                    answer.append("</table>");
                answer.append("<br><table>");
                class_id = names[pos].class_id;
            }
            answer.append(names[pos].answer);
        }
        if (class_id)
            answer.append("</table>");
    }
 
    std::pair <std::vector <position_t> *, size_t> calc_index_3 (std::string_view request)
    {
        std::pair <std::vector <position_t> *, size_t> answer = {nullptr, 0};
        if (is_mb2_begin(request[0]) && is_mb(request[1]))
        {
            auto it = by_2_chars_mb2.find(request[0]);
            if (it != by_2_chars_mb2.end())
                answer.first = &(it->second[static_cast <uint8_t> (request[1] & 0b00111111)]);
            answer.second = 2;
        }
        else if (is_ascii(request[0]))
        {
            std::pair <uint16_t, size_t> index = calc_index_3_ascii_(request);
            answer.first = &by_2_chars_ascii[index.first];
            answer.second = index.second;
        }
        return answer;
    }
    
    static std::pair <uint16_t, size_t> calc_index_3_ascii_ (std::string_view request)
    {
        std::pair <uint16_t, size_t> answer;
        answer.first = static_cast <size_t> (request[0]) << 7;
        if (is_ascii(request[1]))
        {
            answer.second = 2;
            answer.first += request[1];
        }
        else if (request.size() >= 3 && is_mb_begin(request[1]) && is_mb(request[2]))
        {
            answer.second = 1;
            answer.first += request[2] & 0b00111111;
        }
        return answer;
    }
    
    static uint32_t calc_index_4 (std::string_view request);
    
    static bool is_ascii (char value)
    {
        return (value & 0b10000000) == 0b00000000;
    }
    
    static bool is_mb_begin (char value)
    {
        return (value & 0b11000000) == 0b11000000 &&
               (value & 0b11111000) != 0b11111000;
    }

    static bool is_mb2_begin (char value)
    {
        return (value & 0b11100000) == 0b11000000;
    }

    static bool is_mb3_begin (char value)
    {
        return (value & 0b11110000) == 0b11100000;
    }

    static bool is_mb4_begin (char value)
    {
        return (value & 0b11111000) == 0b11110000;
    }

    static bool is_mb (char value)
    {
        return (value & 0b11000000) == 0b10000000;
    }
};


#endif

