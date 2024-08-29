#ifndef FILE_CACHER_H
#define FILE_CACHER_H

#include <unordered_map>
#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <format>
#include <vector>
#include <iostream>

#include "simple_string.h"

struct file_cacher
{
    void add_file (std::string_view _path)
    {
        try
        {
            std::filesystem::path path("files");
            path += _path;
            if (std::filesystem::file_size(path) > size_limit)
                return;
            std::filesystem::file_time_type mtime = std::filesystem::last_write_time(path);
            
            std::ifstream file(path);
            std::streamsize size = 0;
            file_content cur_file;
            cur_file.etag = std::format("{}", mtime);
            while (file.good())
            {
                char buffer[4096];
                file.read(buffer, sizeof(buffer));
                std::streamsize rb = file.gcount();
                size += rb;
                if (size > static_cast <std::streamsize> (size_limit)) [[unlikely]]
                    return;
                cur_file.data.append(buffer, rb);
            }
            cur_file.headers.append(std::to_string(size))
                            .append("\r\nEtag: ")
                            .append(cur_file.etag)
                            .append("\r\n\r\n");
            answer.emplace(_path, cur_file);
        }
        catch (...)
        {}
    }
    
    bool response (simple_string & response, std::string_view path, std::vector <std::pair <std::string_view, std::string_view> > const & headers)
    {
        std::unordered_map <std::string_view, file_content> ::iterator it = answer.find(path);
        if (it == answer.end())
            return 0;
        try
        {
            response.clear();
            for (auto const & header : headers)
            {
                if (header.first == "If-None-Match" && header.second == it->second.etag)
                {
                    response.append("HTTP/1.1 304 Not Modified\r\nContent-Length: 0\r\n\r\n");
                    return 1;
                }
            }
            response.reserve(head.size() + it->second.headers.size() + it->second.data.size());
            response.append(head, it->second.headers, it->second.data);
            return 1;
        }
        catch (...)
        {
            return 0;
        }
    }
    
    static const constexpr size_t size_limit = 100 * 1024;
    
    struct file_content
    {
        std::string headers;
        std::string data;
        std::string etag;
    };
    std::unordered_map <std::string_view, file_content> answer; // path ->
    static const constexpr std::string_view head =  "HTTP/1.1 200 OK\r\n"
                                                    "Content-Length: ";
};

#endif

