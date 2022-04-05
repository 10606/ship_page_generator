#ifndef TABLE_H
#define TABLE_H

#include <string_view>

struct table
{
    static constexpr std::string_view begin = "<table border=1><td>\n";
    static constexpr std::string_view end = "</td></table>\n";
    
    static constexpr std::string_view new_column = "</td>\n<td>\n";
    static constexpr std::string_view new_row = "</td></tr>\n<tr><td>\n";
    static constexpr std::string_view new_line = "<br>\n";
};

#endif
