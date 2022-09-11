#ifndef APPEND_ROW_H
#define APPEND_ROW_H

#include <string>
#include <vector>
#include "table.h"
#include "simple_string.h"


template <typename text, std::string text::* member, typename partial>
inline void append_row (simple_string & answer, std::vector <partial> const & list, std::vector <text> const & text_cache)
{
    for (partial const & item : list)
        answer.append(text_cache[item.index].*member);
    answer.append(table::new_row);
}


#endif

