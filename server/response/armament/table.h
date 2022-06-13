#ifndef TABLE_H
#define TABLE_H

#include <string_view>

struct table
{
    static constexpr std::string_view begin = 
    "<style type = \"text/css\"> \n\
        table { table-layout: fixed; } \n\
        table TD \n\
        { \n\
            min-width: 150px; \n\
            max-width: 150px; \n\
        } \n\
    </style>\n\
    <table border=1><td>\n";
    static constexpr std::string_view end = "</td></table><br>\n";
    
    static constexpr std::string_view new_column = "</td>\n<td>\n";
    static constexpr std::string_view new_row = "</td></tr>\n<tr><td>\n";
    static constexpr std::string_view new_line = "<br>\n";
};

#endif

