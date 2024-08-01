#ifndef BUFFER_H
#define BUFFER_H

#include <algorithm>
#include <span>
#include <cstring>
#include <iostream>


struct buffer_t
{
    constexpr buffer_t () noexcept :
        data(nullptr),
        offset(0),
        _size(0),
        capacity(0),
        total_offset(0)
    {}

    constexpr buffer_t (std::string_view value) noexcept :
        data(nullptr),
        offset(0),
        _size(value.size()),
        capacity(_size),
        total_offset(0)
    {
        data = new char [value.size()];
        std::copy(value.begin(), value.end(), data);
    }

    // owned !!
    constexpr buffer_t (char * _data, size_t __size) noexcept :
        data(_data),
        offset(0),
        _size(__size),
        capacity(_size),
        total_offset(0)
    {}

    constexpr buffer_t (buffer_t && other) noexcept :
        data(other.data),
        offset(other.offset),
        _size(other._size),
        capacity(other.capacity),
        total_offset(other.total_offset)
    {
        other.clear_deleted();
    }
    
    constexpr buffer_t & operator = (buffer_t && other) noexcept
    {
        delete [] data;
        data = other.data;
        offset = other.offset;
        _size = other._size;
        capacity = other.capacity;
        total_offset = other.total_offset;
        
        other.clear_deleted();
        return *this;
    }
    
    constexpr ~buffer_t () noexcept
    {
        clear();
    }
    
    constexpr void clear () noexcept
    {
        delete [] data;
        clear_deleted();
    }

    constexpr buffer_t (buffer_t const &) = delete;
    constexpr buffer_t & operator = (buffer_t const &) = delete;

    constexpr std::span <char> get_buffer ()
    {
        // don't eat all capacity
        if (_size + 1 >= capacity)
            realloc();
        if (offset == 0)
            return {data + _size, capacity - _size - 1};
        else if (offset + _size < capacity)
            return {data + offset + _size, capacity - offset - _size};
        else
            return {data + _size + offset - capacity, capacity - _size - 1};
    }

    constexpr void written (size_t count) noexcept
    {
        _size += count;
    }

    constexpr std::span <char> get_data () noexcept
    {
        return {data + offset, (offset + _size < capacity)? _size : capacity - offset};
    }
    
    constexpr void readed (size_t count) noexcept
    {
        if (offset + count < capacity) [[likely]]
            offset += count;
        else
            offset = offset + count - capacity;
        _size -= count;
        total_offset += count;
    }

    
    constexpr void realloc ()
    {
        size_t new_capacity = capacity * 2 + 4096;
        char * new_data = new char [new_capacity];
        if (data)
        {
            size_t first_part = std::min(capacity - offset, _size);
            std::memcpy(new_data, data + offset, first_part);
            if (first_part != _size)
                std::memcpy(new_data + first_part, data, _size - first_part);
            delete [] data;
        }
        capacity = new_capacity;
        data = new_data;
        offset = 0;
    }

    constexpr size_t size () const noexcept
    {
        return _size;
    }
    
    constexpr bool empty () const noexcept
    {
        return _size == 0;
    }
    
    
    using iterator = char *;
    using const_iterator = char const *;

    constexpr iterator begin () noexcept
    {
        return data + offset;
    }

    constexpr iterator end () noexcept
    {
        if (offset + _size < capacity)
            return data + offset + _size;
        else
            return data + (offset + _size - capacity);
    }
    
    constexpr const_iterator cbegin () const noexcept
    {
        return data + offset;
    }

    constexpr const_iterator cend () const noexcept
    {
        if (offset + _size < capacity)
            return data + offset + _size;
        else
            return data + (offset + _size - capacity);
    }

    constexpr const_iterator begin () const noexcept
    {
        return cbegin();
    }

    constexpr const_iterator end () const noexcept
    {
        return cend();
    }
    
    
    struct overflow_iterator
    {
        constexpr overflow_iterator (char * _data, size_t _pos, size_t _capacity) :
            data(_data),
            pos(_pos),
            capacity(_capacity)
        {}

        constexpr overflow_iterator (buffer_t * _buf, size_t _pos) :
            data(_buf->data),
            pos(_pos),
            capacity(_buf->capacity)
        {}

        constexpr overflow_iterator (buffer_t & _buf, iterator it) :
            data(_buf.data),
            pos(it - _buf.data),
            capacity(_buf.capacity)
        {}
        
        constexpr iterator unsafe () noexcept
        {
            if (!data) [[unlikely]]
                return nullptr;
            return &operator * ();
        }

        using difference_type = size_t;
        using value_type = char;
        using pointer = char *;
        using reference = char &;
        using iterator_category = std::random_access_iterator_tag;
        
        constexpr char & operator * () noexcept
        {
            return data[pos];
        }
        
        constexpr overflow_iterator & operator ++ () noexcept
        {
            pos++;
            if (pos >= capacity) [[unlikely]]
                pos -= capacity;
            return *this;
        }
    
        constexpr overflow_iterator & operator -- () noexcept
        {
            pos += capacity - 1;
            if (pos >= capacity) [[likely]]
                pos -= capacity;
            return *this;
        }
    
        constexpr overflow_iterator operator ++ (int) noexcept
        {
            overflow_iterator answer = *this;
            operator ++ ();
            return answer;
        }
    
        constexpr overflow_iterator operator -- (int) noexcept
        {
            overflow_iterator answer = *this;
            operator -- ();
            return answer;
        }

        constexpr overflow_iterator & operator += (size_t diff /* < capacity */) noexcept
        {
            pos += diff;
            if (pos >= capacity) [[unlikely]]
                pos -= capacity;
            return *this;
        }
    
        constexpr overflow_iterator & operator -= (size_t diff /* < capacity */) noexcept
        {
            pos += capacity - diff;
            if (pos >= capacity) [[likely]]
                pos -= capacity;
            return *this;
        }
    
        constexpr overflow_iterator operator + (size_t diff /* < capacity */) const noexcept
        {
            overflow_iterator answer = *this;
            answer += diff;
            return answer;
        }
    
        constexpr overflow_iterator operator - (size_t diff /* < capacity */) const noexcept
        {
            overflow_iterator answer = *this;
            answer -= diff;
            return answer;
        }
        
        constexpr char & operator [] (size_t diff) noexcept
        {
            diff += pos;
            if (diff >= capacity) [[unlikely]]
                diff -= capacity;
            return data[diff];
        }
    
        constexpr size_t operator - (overflow_iterator const & other) noexcept
        {
            if (pos >= other.pos) [[likely]]
                return pos - other.pos;
            return pos + capacity - other.pos;
        }
    
        
        constexpr std::strong_ordering operator <=> (overflow_iterator const &) const noexcept = default;

        constexpr bool safe_range_with (overflow_iterator const & other) const noexcept
        {
            return pos <= other.pos;
        }
    
        char * data;
        size_t pos;
        size_t capacity;
    };

    constexpr overflow_iterator overflow_begin () noexcept
    {
        return {this, offset};
    }

    constexpr overflow_iterator overflow_end () noexcept
    {
        return {this, (offset + _size) % capacity};
    }
    

    struct safe_iterator
    {
        constexpr safe_iterator (buffer_t * _buf, size_t _pos) :
            buf(_buf),
            pos(_pos)
        {}

        constexpr safe_iterator (buffer_t & _buf, iterator it) :
            buf(&_buf),
            pos(0)
        {
            pos = it - buf->data - buf->offset;
            if (buf->data + buf->offset > it)
                pos += buf->capacity;
            pos += buf->total_offset;
        }

        constexpr safe_iterator (buffer_t & _buf, overflow_iterator it) :
            safe_iterator(_buf, it.unsafe())
        {}
        
        constexpr iterator unsafe () noexcept
        {
            if (!buf->data) [[unlikely]]
                return nullptr;
            return &operator * ();
        }

        constexpr overflow_iterator overflow () noexcept
        {
            return overflow_iterator(*buf, unsafe());
        }

        using difference_type = size_t;
        using value_type = char;
        using pointer = char *;
        using reference = char &;
        using iterator_category = std::random_access_iterator_tag;
        
        constexpr char & operator * () noexcept
        {
            size_t data_pos = pos + buf->offset - buf->total_offset;
            if (data_pos >= buf->capacity)
                data_pos -= buf->capacity;
            return buf->data[data_pos];
        }
        
        constexpr safe_iterator & operator ++ () noexcept
        {
            pos++;
            return *this;
        }
    
        constexpr safe_iterator & operator -- () noexcept
        {
            pos--;
            return *this;
        }
    
        constexpr safe_iterator operator ++ (int) noexcept
        {
            safe_iterator answer = *this;
            pos++;
            return answer;
        }
    
        constexpr safe_iterator operator -- (int) noexcept
        {
            safe_iterator answer = *this;
            pos--;
            return answer;
        }

        constexpr safe_iterator & operator += (size_t diff) noexcept
        {
            pos += diff;
            return *this;
        }
    
        constexpr safe_iterator & operator -= (size_t diff) noexcept
        {
            pos -= diff;
            return *this;
        }
    
        constexpr safe_iterator operator + (size_t diff) const noexcept
        {
            safe_iterator answer = *this;
            answer.pos += diff;
            return answer;
        }
    
        constexpr safe_iterator operator - (size_t diff) const noexcept
        {
            safe_iterator answer = *this;
            answer.pos -= diff;
            return answer;
        }
        
        constexpr char & operator [] (size_t diff) noexcept
        {
            size_t data_pos = pos + diff + buf->offset - buf->total_offset;
            if (data_pos >= buf->capacity)
                data_pos -= buf->capacity;
            return buf->data[data_pos];
        }
    
        constexpr size_t operator - (safe_iterator const & other) noexcept
        {
            return pos - other.pos;
        }
    
        
        constexpr std::strong_ordering operator <=> (safe_iterator const &) const noexcept = default;
    
        buffer_t * buf;
        size_t pos;
    };
    
    constexpr safe_iterator safe_begin () noexcept
    {
        return {this, total_offset};
    }

    constexpr safe_iterator safe_end () noexcept
    {
        return {this, total_offset + _size};
    }
    

    constexpr void clear_deleted ()
    {
        data = nullptr;
        offset = 0;
        _size = 0;
        capacity = 0;
        total_offset = 0;
    }

    char * data;
    size_t offset;
    size_t _size;
    size_t capacity; // capacity == 0 || size + 1 <= capacity
    size_t total_offset; // for non invalid iterators
};

inline std::pair <buffer_t::iterator, buffer_t::iterator> first_part (buffer_t::overflow_iterator begin, buffer_t::overflow_iterator end) noexcept
{
    if (begin.safe_range_with(end))
        return {begin.unsafe(), end.unsafe()};
    return {begin.unsafe(), begin.data + begin.capacity};
}

inline std::pair <buffer_t::iterator, buffer_t::iterator> second_part (buffer_t::overflow_iterator begin, buffer_t::overflow_iterator end) noexcept
{
    if (begin.safe_range_with(end))
        return {end.unsafe(), end.unsafe()};
    return {end.data, end.unsafe()};
}

inline buffer_t::overflow_iterator find (buffer_t::overflow_iterator begin, buffer_t::overflow_iterator end, char value) noexcept
{
    std::pair <buffer_t::iterator, buffer_t::iterator> first = first_part(begin, end);
    buffer_t::iterator it_in_first = std::find(first.first, first.second, value);
    if (it_in_first != first.second)
        return buffer_t::overflow_iterator(begin.data, it_in_first - begin.data, begin.capacity);

    std::pair <buffer_t::iterator, buffer_t::iterator> second = second_part(begin, end);
    buffer_t::iterator it_in_second = std::find(second.first, second.second, value);
    return buffer_t::overflow_iterator(begin.data, it_in_second - begin.data, begin.capacity);
}

#endif

