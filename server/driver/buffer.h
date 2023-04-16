#ifndef BUFFER_H
#define BUFFER_H

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
        if (_size == capacity)
            realloc();
        if (offset + _size < capacity)
            return {data + offset + _size, capacity - offset - _size};
        else
            return {data + _size + offset - capacity, capacity - _size};
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
        if (offset + count < capacity)
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
        total_offset += offset;
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
        
        constexpr iterator unsafe () noexcept
        {
            if (!buf->data)
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
    
        constexpr safe_iterator operator + (size_t diff) noexcept
        {
            safe_iterator answer = *this;
            pos += diff;
            return answer;
        }
    
        constexpr safe_iterator operator - (size_t diff) noexcept
        {
            safe_iterator answer = *this;
            pos -= diff;
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
        return {this, total_offset + offset};
    }

    constexpr safe_iterator safe_end () noexcept
    {
        return {this, total_offset + offset + _size};
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
    size_t capacity;
    size_t total_offset; // for non invalid iterators
};

#endif

