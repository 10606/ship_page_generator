#ifndef GET_SEGMENTS_H
#define GET_SEGMENTS_H

#include <vector>
#include <optional>
#include <algorithm>

template <typename key_t, typename value_t>
struct get_segments;


template <typename key_t, typename value_t>
struct get_segments <std::optional <key_t>, value_t>
{
    struct data_t
    {
        std::optional <key_t> begin;
        std::optional <key_t> end;
        value_t value;
    };

    get_segments (std::vector <data_t> values)
    {
        data.emplace_back();

        for (data_t const & value : values)
        {
            if (value.begin)
                data.emplace_back(*value.begin, std::vector <value_t> ());
            if (value.end)
                data.emplace_back(*value.end, std::vector <value_t> ());
        }

        std::sort
        (
            data.begin() + 1,
            data.end(),
            [] (std::pair <key_t, value_list> const & a, std::pair <key_t, value_list> const & b) -> bool
            { return a.first < b.first; }
        );
        it_t it = std::unique
        (
            data.begin() + 1,
            data.end(),
            [] (std::pair <key_t, value_list> const & a, std::pair <key_t, value_list> const & b) -> bool
            { return a.first == b.first; }
        );
        data.erase(it, data.end());

        data.emplace_back();

        for (data_t const & value : values)
        {
            it_t begin = data.begin();
            if (value.begin)
                begin = std::lower_bound(data.begin() + 1, data.end() - 1, *value.begin, comparator_ba);

            it_t end = data.end();
            if (value.end)
                end = std::lower_bound(data.begin() + 1, data.end() - 1, *value.end, comparator_ba);

            for (; begin != end; ++begin)
                begin->second.push_back(value.value);
        }
    }

    typedef std::vector <value_t> value_list;

    value_list const & get (key_t key) const noexcept
    {
        cit_t answer = std::upper_bound(data.begin() + 1, data.end() - 1, key, comparator_ab) - 1;
        return answer->second;
    }

    value_list const & get (std::optional <key_t> key) const noexcept
    {
        if (!key)
            return data.back().second;
        return get(*key);
    }

private:
    std::vector <std::pair <key_t, value_list> > data; // with 2 fakes: begin, end

    typedef typename std::vector <std::pair <key_t, value_list> > ::iterator it_t;
    typedef typename std::vector <std::pair <key_t, value_list> > ::const_iterator cit_t;

    static const constexpr auto comparator_ab = [] (key_t const & a, std::pair <key_t, value_list> const & b) -> bool
                                             { return a < b.first; };
    static const constexpr auto comparator_ba = [] (std::pair <key_t, value_list> const & a, key_t const & b) -> bool
                                             { return a.first < b; };
};


#endif

