#ifndef SEARCH_H
#define SEARCH_H

#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include "ship_names_list.h"
#include "ship_requests.h"
#include "ship_info.h"
#include "simple_string.h"
#include "armament_links.h"
#include "parse_query.h"


struct search
{
    search (ship_requests *, ship_names_list const & _ship_names) :
        ship_names(_ship_names)
    {
        std::vector <ship_names_list::ship_info_t> const & names = ship_names.names();
        for (size_t i = 0; i != names.size(); ++i)
        {
            add(names[i].name_ru, i);
            add(names[i].name_en, i);
        }
    }
    
    void response (simple_string & answer, std::string_view request, piece_t title);

    static std::string get_search_parameter (std::string_view request);
    
private:
    ship_names_list const & ship_names;
    
    static const constexpr std::string_view search_keyword = "search=";

    friend struct add_ship_t;
    typedef ship_requests::ship_info_t::list ship_info_long;

    struct position_t
    {
        uint32_t index;
        uint32_t offset;
    };
    
    std::array <std::vector <uint32_t>, 256> by_1_chars;
    std::unordered_map <uint8_t, std::array <std::vector <position_t>, (1lu << 6)> > by_2_chars_mb2; // multi_byte 2
    std::array <std::vector <position_t>, (1lu << (7 + 7))> by_2_chars_ascii; // ascii | acsii,  ascii | mb <n> here use second char
    std::unordered_map <uint32_t, std::vector <position_t> > by_4_chars; // last chars for multi byte
    
    void add (std::string_view value, size_t name_index);
    
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
            answer.first += static_cast <uint8_t> (request[1]);
        }
        else if (request.size() >= 3 && is_mb_begin(request[1]) && is_mb(request[2]))
        {
            answer.second = 1;
            answer.first += static_cast <uint8_t> (request[2]) & 0b00111111;
        }
        return answer;
    }
    
    static uint32_t calc_index_4 (std::string_view request, size_t use_symbols = 4);
    
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

