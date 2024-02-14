#ifndef CONNECTION_HELPER_H
#define CONNECTION_HELPER_H

#include <algorithm>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "buffer.h"


struct safe_view
{
    constexpr safe_view
    (
        buffer_t::safe_iterator _begin,
        buffer_t::safe_iterator _end
    ) :
        begin(_begin),
        end(_end)
    {}

    constexpr void set_buffer (buffer_t & buf) noexcept
    {
        begin.buf = &buf;
        end.buf = &buf;
    }

    std::string_view to_string_view (std::string & overflow_case)
    {
        if (begin.unsafe() <= end.unsafe()) [[likely]]
            return std::string_view(begin.unsafe(), end.unsafe());
        
        std::pair <buffer_t::iterator, buffer_t::iterator> first = first_part(begin.overflow(), end.overflow());
        std::pair <buffer_t::iterator, buffer_t::iterator> second = second_part(begin.overflow(), end.overflow());
        overflow_case = std::string(first.first, first.second)
                            .append(second.first, second.second);
        return overflow_case;
    }
    
    buffer_t::safe_iterator begin;
    buffer_t::safe_iterator end;
};

struct header_t
{
    header_t
    (
        safe_view _key,
        safe_view _value
    ) :
        key(_key),
        value(_value)
    {}

    std::pair <std::string_view, std::string_view> to_string_view (std::string & overflow_case)
    {
        return {key.to_string_view(overflow_case), value.to_string_view(overflow_case)};
    }

    safe_view key;
    safe_view value;
};

struct content_range_t
{
    content_range_t (std::vector <header_t> & headers) :
        start(0),
        size()
    {
        std::string overflow_case; // we can get overflow only once
        for (header_t & header : headers)
        {
            off_t answer_start = 0;
            std::optional <off_t> answer_size;
            if (header.key.to_string_view(overflow_case) != "Content-Range")
                continue;
            std::string_view value = header.value.to_string_view(overflow_case);
            static const std::string_view unit = "bytes";
            auto [etalon_read, parsed] =
                std::mismatch(unit.begin(), unit.end(), value.begin(), value.end());
            if (etalon_read != unit.end())
                continue;
            
            auto skip_white_spaces = [&parsed, end = value.end()] () -> void 
            {
                while (parsed != end && std::isspace(*parsed))
                    parsed++;
            };
            
            skip_white_spaces();
            if (parsed == value.end())
                continue;
            if (*parsed == '*')
            {
                parsed++;
                start = 0;
            }
            else
            {
                off_t range_begin_value;
                std::from_chars_result range_begin = std::from_chars(parsed, value.end(), range_begin_value);
                if (range_begin.ec != std::errc())
                    continue;
                parsed = range_begin.ptr;
                skip_white_spaces();
                if (parsed == value.end() || *parsed != '-')
                    continue;
                parsed++;
                if (parsed == value.end())
                    continue;
                skip_white_spaces();
                off_t range_end_value;
                std::from_chars_result range_end = std::from_chars(parsed, value.end(), range_end_value);
                if (range_end.ec != std::errc())
                    continue;
                parsed = range_end.ptr;
                if (range_begin_value < 0 || range_end_value < 0 || range_begin_value > range_end_value)
                    continue;
                answer_start = range_begin_value;
                answer_size = range_end_value - range_begin_value + 1;
            }
            skip_white_spaces();
            if (parsed == value.end() || *parsed != '/')
                continue;
            parsed++;
            if (parsed == value.end())
                continue;
            skip_white_spaces();
            if (*parsed == '*')
            {
                parsed++;
            }
            else
            {
                off_t size_value;
                std::from_chars_result range_begin = std::from_chars(parsed, value.end(), size_value);
                if (range_begin.ec != std::errc())
                    continue;
                parsed = range_begin.ptr;
                /*
                if (answer_size && *answer_size != size_value)
                    continue;
                answer_size = size_value;
                */
            }
            skip_white_spaces();
            if (parsed != value.end())
                continue;
            start = answer_start;
            size = answer_size;
        }
    }
    
    off_t start;
    std::optional <off_t> size;
};

#endif

