#ifndef LAST_ACTIVE_H
#define LAST_ACTIVE_H

#include <stdint.h>
#include <stddef.h>
#include <map>
#include <chrono>
#include <optional>


struct last_active_t
{
    void insert (int fd)
    {
        try
        {
            auto it = from_time.insert({std::chrono::system_clock::now(), fd});
            try
            {
                from_fd.insert({fd, it});
            }
            catch (...)
            {
                from_time.erase(it);
            }
        }
        catch (...)
        {}
    }
 
    void update (int fd)
    {
        erase(fd);
        insert(fd);
    }
    
    void erase (int fd)
    {
        auto it = from_fd.find(fd);
        if (it == from_fd.end())
            return;
        from_time.erase(it->second);
        from_fd.erase(it);
    }
    
    template <typename Rep, typename Period>
    std::optional <int> get_inactive (std::chrono::duration <Rep, Period> time_limit) const
    {
        if (from_time.empty())
            return std::nullopt;
        std::chrono::time_point <std::chrono::system_clock> now = std::chrono::system_clock::now();
        if (now - from_time.begin()->first > time_limit)
            return from_time.begin()->second;
        return std::nullopt;
    }

    std::multimap <std::chrono::time_point <std::chrono::system_clock>, int> from_time;
    std::map <int, decltype(from_time)::iterator> from_fd;
};


#endif

