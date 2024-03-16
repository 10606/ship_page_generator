#ifndef APPEND_ROW_H
#define APPEND_ROW_H

#include <string>
#include <vector>
#include "table.h"
#include "simple_string.h"


template <typename text, std::string text::* member, bool place_delimeter = 1, typename partial>
inline void append_row (simple_string & answer, std::vector <partial> const & list, std::vector <text> const & text_cache)
{
    size_t size = answer.size() + table::new_row.size();
    for (partial const & item : list)
        size += (text_cache[item.index].*member).size();
    answer.reserve(size);
    
    for (partial const & item : list)
        answer.append_without_realloc(text_cache[item.index].*member);
    if constexpr (place_delimeter)
        answer.append_without_realloc(table::new_row);
}


#endif

