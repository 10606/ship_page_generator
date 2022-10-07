#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <vector>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <chrono>
#include <optional>


struct sunk_and_next_day
{
    std::optional <std::chrono::year_month_day> sunk;
    std::optional <std::chrono::year_month_day> next;
};
typedef std::unordered_map <int, sunk_and_next_day> sunk_dates_t;
sunk_dates_t const & sunk_dates ();

template 
<
    typename T,
    typename ship_item_raw_t,
    typename item_raw_t,
    std::vector <typename T::p_response_t> T::* items,
    int ship_item_raw_t::* item_id_ptr,
    typename compare_gen_t,
    typename ... additional_args_t
>
void fill_data_structures
(
    T & object,
    std::vector <item_raw_t> const & items_raw,
    std::vector <ship_item_raw_t> && items_on_ship_raw,
    std::unordered_map <int, std::vector <typename T::ship_items_lt> > * items_on_ship,
    compare_gen_t && compare_gen,
    additional_args_t && ... additional_args
)
{
    std::set <int> used; // add only used
    for (ship_item_raw_t const & item : items_on_ship_raw)
        used.insert(item.*item_id_ptr);
    
    std::unordered_map <int, size_t> items_index;
    (object.*items).reserve(used.size());
    std::vector <size_t> old_index;
    old_index.reserve(used.size());
    for (size_t i = 0; i != items_raw.size(); ++i)
    {
        item_raw_t const & item = items_raw[i];
        int item_id = item.id;
        if (used.find(item_id) == used.end())
            continue;
        items_index.insert({item_id, (object.*items).size()});
        (object.*items).push_back(object.partial_response(item, std::forward <additional_args_t> (additional_args) ...));
        old_index.push_back(i);
    }
    
    sunk_dates_t const & sunk_date = sunk_dates ();
    for (ship_item_raw_t & item : items_on_ship_raw)
    {
        std::unordered_map <int, size_t> ::iterator it = items_index.find(item.*item_id_ptr);
        sunk_dates_t::const_iterator sunk = sunk_date.find(item.ship_id);
        if (it != items_index.end())
        {
            if (sunk != sunk_date.end() && item.date_to && sunk->second.sunk == item.date_to)
                item.date_to = sunk->second.next;
            (*items_on_ship)[item.ship_id].emplace_back(it->second, item);
        }
    }
    
    std::invoke_result_t <compare_gen_t, std::vector <item_raw_t> const &, std::vector <size_t> const &> compare =
        std::forward <compare_gen_t> (compare_gen)(items_raw, old_index);
    for (auto & item : *items_on_ship)
        std::sort(item.second.begin(), item.second.end(), compare);
}

#endif

