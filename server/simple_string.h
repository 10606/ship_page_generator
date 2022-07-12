#ifndef SIMPLE_STRING_H
#define SIMPLE_STRING_H

#include <stddef.h>
#include <string_view>
#include <stdlib.h>
#include <cstring>
#include <stdexcept>


struct simple_string
{
    simple_string () noexcept :
        _data(nullptr),
        _size(0),
        capacity(0)
    {}

    simple_string (simple_string const &) = delete;
    simple_string (simple_string &&) = delete;
    simple_string & operator = (simple_string const &) = delete;
    simple_string & operator = (simple_string &&) = delete;

    ~simple_string () noexcept
    {
        free(_data);
    }
    
    void reserve (size_t value)
    {
        if (capacity < value)
            realloc(value);
    }

    unsigned char const * data () const noexcept
    {
        return _data;
    }
    
    unsigned char * reset () noexcept
    {
        _size = 0;
        capacity = 0;
        unsigned char * answer = _data;
        _data = nullptr;
        return answer;
    }

    simple_string & append (std::string_view value)
    {
        if (value.size() + _size > capacity)
            realloc(3 * value.size() + _size * 4 + 64);
        memcpy(_data + _size, value.data(), value.size());
        _size += value.size();
        return *this;
    }

    simple_string & append (char const * value)
    {
        return append(std::string_view{value, strlen(value)});
    }
    
    simple_string & append (std::string const & value)
    {
        return append(std::string_view(value));
    }
    
    size_t size () const noexcept
    {
        return _size;
    }
    
    void rewrite (size_t pos, std::string_view value) noexcept
    {
        if (pos + value.size() > _size)
            return;
        memcpy(_data + pos, value.data(), value.size());
    }

private:
    unsigned char * _data;
    size_t _size;
    size_t capacity;
    
    void realloc (size_t value)
    {
        unsigned char * new_data = static_cast <unsigned char *> (malloc(value));
        if (!new_data)
            throw std::runtime_error("can't alloc");
        if (_size > 0)
            std::memcpy(new_data, _data, _size);
        free(_data);
        _data = new_data;
        capacity = value;
    }
};


#endif

