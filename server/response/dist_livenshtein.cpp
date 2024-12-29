#include "dist_livenshtein.h"

#include <array>
#include <limits>
#include <iostream>
#include <stdint.h>

namespace
{

consteval std::array <char, 256> low_cost_transform ()
{
    std::array <char, 256> answer;
    for (size_t i = 0; i != answer.size(); ++i)
    {
        answer[i] = i;
        if (i >= 'A' && i <= 'Z') // std::isupper not constexpr
            answer[i] = i - 'A' + 'a'; // std::tolower not constexpr
    }
    answer['z'] = 's';
    answer['d'] = 't';
    return answer;
}

size_t cost (char a, char b)
{
    if (a == b) [[unlikely]]
        return 0;
    static const constexpr std::array <char, 256> low_cost_transform_table = low_cost_transform();
    if (low_cost_transform_table[reinterpret_cast <uint8_t &> (a)] ==
        low_cost_transform_table[reinterpret_cast <uint8_t &> (b)])
        return 1;
    return 10;
}

}

size_t dist_livenshtein (std::string_view a, std::string_view b)
{
    static const constexpr size_t see_length = 4; // must be > 2

    // cost a -> b
    static const constexpr size_t insert_cost = 15;
    static const constexpr size_t remove_cost = 15;
    static const constexpr size_t insert_begin = 1;
    static const constexpr size_t remove_begin = 15;

    std::array <size_t, see_length> dp_0 = {0, 1 * remove_begin, 2 * remove_begin, 3 * remove_begin};
    std::array <size_t, see_length> dp_1;
    
    
    if (a.size() + ((see_length - 1) / 2) < b.size())
        return std::numeric_limits <size_t> ::max();
    if (b.size() + ((see_length - 0) / 2) < a.size())
        return std::numeric_limits <size_t> ::max();
    if (b.empty())
        return a.size() * remove_begin;
    if (a.empty())
        return b.size() * insert_begin;
    
    if (a.size() < see_length)
    {
        auto fill_direct_level = [&a, &b]
        (std::array <size_t, see_length> & dp_prev, std::array <size_t, see_length> & dp_cur, size_t offset_b) -> void
        {
            dp_cur[0] = dp_prev[0] + insert_cost; // insert
            dp_cur[0] = std::min(dp_cur[0], insert_begin * (offset_b + 1));
            for (size_t i = 0; i != a.size(); ++i)
            {
                dp_cur[i + 1] = std::min(dp_cur[i] + remove_cost, dp_prev[i + 1] + insert_cost); // remove, insert
                dp_cur[i + 1] = std::min(dp_cur[i + 1], dp_prev[i] + cost(a[i], b[offset_b])); // replace
            }
        };
        
        for (size_t j = 0; j + 2 <= b.size(); j += 2)
        {
            fill_direct_level(dp_0, dp_1, j);
            fill_direct_level(dp_1, dp_0, j + 1);
        }
        
        if (b.size() % 2 == 0)
            return dp_0[a.size()];
        else
        {
            fill_direct_level(dp_0, dp_1, b.size() - 1);
            return dp_1[a.size()];
        }
    }
    else
    {
        //           a
        //     - 0 1 2 3 4 5
        //   - p p p p
        //   0 q q q q
        //   1   r r r r
        //   2     s s s s
        // b 3     t t t t
        //   4       u u u u
        //   5       v v v v
        //   6       w w w w

        // p -> q
        // s -> t
        // u -> v
        auto fill_direct_level = [&a, &b] 
        (std::array <size_t, see_length> & dp_prev, std::array <size_t, see_length> & dp_cur, size_t offset_a, size_t offset_b) -> void
        {
            for (size_t i = 0; i != see_length; ++i)
                dp_cur[i] = dp_prev[i] + insert_cost; // insert
            if (offset_a >= a.size()) [[unlikely]]
                dp_cur[0] = std::min(dp_cur[0], insert_begin * (offset_b + 1));
            for (size_t i = 1; i != see_length; ++i)
            {
                dp_cur[i] = std::min(dp_cur[i], dp_cur[i - 1] + remove_cost); // remove
                dp_cur[i] = std::min(dp_cur[i], dp_prev[i - 1] + cost(a[offset_a + i], b[offset_b])); // replace
            }
        };
        
        // q -> r
        // r -> s
        // t -> u
        auto fill_shift_level = [&a, &b]
        (std::array <size_t, see_length> & dp_prev, std::array <size_t, see_length> & dp_cur, size_t offset_a, size_t offset_b) -> void
        {
            auto update_by_diag = [&a, &b, &dp_prev, &dp_cur, offset_a, offset_b] (size_t i) -> void
            {
                dp_cur[i] = std::min(dp_cur[i], dp_prev[i] + cost(a[offset_a + i], b[offset_b])); // replace
            };
            
            dp_cur[0] = dp_prev[1] + insert_cost; // insert
            update_by_diag(0);
            
            for (size_t i = 1; i != see_length - 1; ++i)
            {
                dp_cur[i] = std::min(dp_cur[i - 1] + remove_cost, dp_prev[i + 1] + insert_cost); // remove, insert
                update_by_diag(i);
            }

            {
                size_t i = see_length - 1;
                dp_cur[i] = dp_cur[i - 1] + remove_cost; // remove
                update_by_diag(i);
            }
        };

        auto calc_i = [&a, &b] (size_t j, size_t i_prev) -> size_t
        {
            size_t i_0 = (j + 1) * (a.size() + 1 - see_length) / b.size();
            if (i_prev + 1 < i_0)
                i_0 = i_prev + 1;
            if (i_0 + see_length > a.size() + 1)
                i_0 = a.size() + 1 - see_length;
            return i_0;
        };
        
        size_t i_prev = 0;
        for (size_t j = 0; j + 2 <= b.size(); j += 2)
        {
            size_t i_0 = calc_i(j + 0, i_prev);
            size_t i_1 = calc_i(j + 1, i_0);
            
            if (i_prev == i_0)
                fill_direct_level(dp_0, dp_1, i_0 - 1, j);
            else
                fill_shift_level (dp_0, dp_1, i_0 - 1, j);
            if (i_0 == i_1)
                fill_direct_level(dp_1, dp_0, i_1 - 1, j + 1);
            else
                fill_shift_level (dp_1, dp_0, i_1 - 1, j + 1);
            i_prev = i_1;
        }
        
        if (b.size() % 2 == 0)
            return dp_0[see_length - 1];
        else
        {
            size_t i_last = a.size() + 1 - see_length;
            if (i_prev == i_last)
                fill_direct_level(dp_0, dp_1, i_last - 1, b.size() - 1);
            else
                fill_shift_level(dp_0, dp_1, i_last - 1, b.size() - 1);
            return dp_1[see_length - 1];
        }
    }
}

