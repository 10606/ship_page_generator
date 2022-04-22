#include "validate_path.h"

#include <filesystem>
#include <cctype>
#include <iostream>

bool simple_check (std::string_view path)
{
    if (path.empty() || path.size() > 2000)
        return 0;
    char prev_symbol = 0;
    for (char c : path)
    {
        if (std::isalpha(c) ||
            std::isdigit(c) ||
            c == '_' ||
            c == '-')
        {
            prev_symbol = c;
            continue;
        }
        if (prev_symbol == c)
            return 0;
        if (c != '/' &&
            c != '.')
            return 0;
        prev_symbol = c;
    }
    return 1;
}

std::string filesystem_check (std::string_view path)
{
    try
    {
        if (!simple_check(path))
            return std::string();

        if (path[0] == '/')
            path = path.substr(1);
        
        std::filesystem::path pwd = std::filesystem::current_path();
        pwd.append("files").append(path);
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

