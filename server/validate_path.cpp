#include "validate_path.h"
#include "parse_query.h"

#include <filesystem>
#include <cctype>
#include <iostream>

bool simple_check (std::string_view path)
{
    if (path.empty() || path.size() > 2000)
        return 0;
    char prev_symbol = 0;
    char prev_cnt = 0;
    for (char c : path)
    {
        if (std::isalpha(c) ||
            std::isdigit(c) ||
            c == '%' ||
            c == ' ' ||
            c == '~' ||
            c == ':' ||
            c == '(' ||
            c == ')' ||
            c == '_' ||
            c == '-' ||
            c == '<' ||
            c == '>')
        {
            prev_symbol = c;
            prev_cnt = 1;
            continue;
        }
        if (prev_symbol == c)
        {
            prev_cnt++;
            if (c != '.')
                return 0;
            continue;
        }
        if (c != '/' &&
            c != '.')
            return 0;
        if (c == '/' &&
            prev_symbol == '.' && 
            prev_cnt > 1)
            return 0;
        prev_symbol = c;
        prev_cnt = 1;
    }
    return 1;
}

std::string filesystem_check (std::string_view path)
{
    try
    {
        while (!path.empty() && path[0] == '/')
            path = path.substr(1);
        std::string path_decoded = percent_dec(path, 0);
        
        if (!simple_check(path_decoded))
            return std::string();
        
        std::filesystem::path pwd = std::filesystem::current_path();
        pwd.append("files").append(path_decoded);
        std::filesystem::file_status status = std::filesystem::status(pwd);
        
        if (status.type() != std::filesystem::file_type::regular)
            return std::string();
        
        if ((status.permissions() & std::filesystem::perms::others_read) != 
                                    std::filesystem::perms::others_read)
            return std::string();
        
        return pwd;
    }
    catch (...)
    {
        return std::string();
    }
}

