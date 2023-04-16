#ifndef EPOLL_WRAP_H
#define EPOLL_WRAP_H

#include <stdexcept>
#include <vector>

#include <unistd.h>
#include <sys/epoll.h>


struct epoll_wrap
{
    epoll_wrap () :
        epoll_fd(epoll_create(100))
    {
        if (epoll_fd == -1)
            throw std::runtime_error("can't create epoll");
    }

    ~epoll_wrap ()
    {
        if (epoll_fd != -1)
            close(epoll_fd);
    }
    
    epoll_wrap (epoll_wrap && other) noexcept :
        epoll_fd(other.epoll_fd)
    {
        other.epoll_fd = -1;
    }

    epoll_wrap & operator = (epoll_wrap && other) noexcept
    {
        if (epoll_fd != -1)
            close(epoll_fd);
        epoll_fd = other.epoll_fd;
        other.epoll_fd = -1;
        return *this;
    }
    
    epoll_wrap (epoll_wrap const &) = delete;
    epoll_wrap & operator = (epoll_wrap const &) = delete;
    
    void add (int fd, uint32_t event_mask)
    {
        epoll_event event{.events = event_mask | event_mask_err, .data = {.fd = fd}};
        int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
        if (ret == -1)
            throw std::runtime_error("can't add to epoll");
    }

    void mod (int fd, uint32_t event_mask)
    {
        epoll_event event{.events = event_mask | event_mask_err, .data = {.fd = fd}};
        int ret = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
        if (ret == -1)
            throw std::runtime_error("can't mod in epoll");
    }

    void del (int fd) noexcept
    {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    }
    
    struct envoke
    {
        int fd;
        uint32_t events;
    };
    
    std::vector <envoke> wait (int timeout = -1)
    {
        std::vector <envoke> answer;
        epoll_event events[100];
        answer.reserve(std::extent_v <decltype(events)> / 2);
        int ret = epoll_wait(epoll_fd, events, std::extent_v <decltype(events)>, timeout);
        if (ret == -1)
        {
            if (errno == EINTR)
                return answer;
            throw std::runtime_error("error wait in epoll");
        }
        for (int i = 0; i != ret; ++i)
            answer.push_back({events[i].data.fd, events[i].events});
        return answer;
    }

    template <size_t size>
    size_t wait (envoke (& answer) [size], int timeout = -1)
    {
        epoll_event events[size];
        int ret = epoll_wait(epoll_fd, events, std::extent_v <decltype(events)>, timeout);
        if (ret == -1)
        {
            if (errno == EINTR)
                return 0;
            throw std::runtime_error("error wait in epoll");
        }
        for (int i = 0; i != ret; ++i)
            answer[i] = {events[i].data.fd, events[i].events};
        return ret;
    }

    static const constexpr uint32_t event_mask_err = EPOLLRDHUP | EPOLLERR | EPOLLHUP;

private:
    int epoll_fd;
};


#endif

