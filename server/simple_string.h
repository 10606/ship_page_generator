#ifndef SIMPLE_STRING_CHAR_H
#define SIMPLE_STRING_CHAR_H

#include <concepts>
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
    simple_string & operator = (simple_string const &) = delete;
    
    simple_string (simple_string && other) noexcept :
        _data(other._data),
        _size(other._size),
        capacity(other.capacity)
    {
        other._data = nullptr;
        other._size = 0;
        other.capacity = 0;
    }
    
    simple_string & operator = (simple_string && rhs) noexcept
    {
        if (&rhs == this)
            return *this;
        
        delete [] _data;
        _data = rhs._data;
        _size = rhs._size;
        capacity = rhs.capacity;
        
        rhs._data = nullptr;
        rhs._size = 0;
        rhs.capacity = 0;
        return *this;
    }

    ~simple_string () noexcept
    {
        delete [] _data;
    }
    
    void reserve (size_t value)
    {
        if (capacity < value)
            realloc(value);
    }

    char const * data () const noexcept
    {
        return _data;
    }
    
    char * data () noexcept
    {
        return _data;
    }
    
    char * reset () noexcept
    {
        _size = 0;
        capacity = 0;
        char * answer = _data;
        _data = nullptr;
        return answer;
    }

    simple_string & append (std::string_view value)
    {
        realloc_if_need(value.size());
        append_without_realloc(value);
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
    
    template <typename ... T>
    requires requires
    {
        (std::convertible_to <T, std::string_view> && ...);
    }
    simple_string & append (T && ... values)
    {
        size_t sum_sizes = (std::string_view(values).size() + ...);
        realloc_if_need(sum_sizes);
        (append_without_realloc(std::forward <T> (values)), ...);
        return *this;
    }
    
    size_t size () const noexcept
    {
        return _size;
    }
    
    void rewrite (size_t pos, std::string_view value) noexcept
    {
        if (value.empty() || pos + value.size() > _size) [[unlikely]]
            return;
        memcpy(_data + pos, value.data(), value.size());
    }
    
    // DANGER FUNCTION
    void append_without_realloc (std::string_view value) noexcept
    {
        if (!value.empty()) [[likely]]
            memcpy(_data + _size, value.data(), value.size());
        _size += value.size();
    }
    
    void clear ()
    {
        _size = 0;
    }

private:
    char * _data;
    size_t _size;
    size_t capacity;
    
    void realloc (size_t value)
    {
        char * new_data = new char [value];
        if (_size > 0) [[likely]]
            std::memcpy(new_data, _data, _size);
        delete [] _data;
        _data = new_data;
        capacity = value;
    }
    
    void realloc_if_need (size_t sum_sizes)
    {
        if (sum_sizes + _size > capacity) [[unlikely]]
            realloc(3 * sum_sizes + _size * 4 + 64);
    }
};


template <typename T>
requires requires 
{
    {std::numeric_limits <T> ::digits10} -> std::convertible_to <size_t>;
}
struct number_holder
{
    number_holder () :
        size(0)
    {}
    
    char data[std::numeric_limits <T> ::digits10 + 2]; // 1 most significant + \0
    size_t size;
    
    operator std::string_view ()
    {
        return std::string_view(data, size);
    }
};


#endif


