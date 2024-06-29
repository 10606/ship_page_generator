#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H

#include <stddef.h>
#include <memory>
#include <array>
#include <cstring>
#include <new>
#include <type_traits>
#include <iostream>


namespace allocators
{

    inline size_t read_size (char * where) noexcept
    {
        size_t answer;
        memcpy(&answer, where, sizeof(size_t));
        return answer;
    }
    
    inline char * read_next (char * where /* auto add sizeof(size_t) */) noexcept
    {
        char * answer;
        memcpy(&answer, where + sizeof(size_t), sizeof(char *));
        return answer;
    }
    
    inline void write_size (char * where, size_t value) noexcept
    {
        new (where) size_t(value);
    }

    inline void write_next (char * where /* auto add sizeof(size_t) */, char * value) noexcept
    {
        memcpy(where + sizeof(size_t), &value, sizeof(char *));
    }
    
}


struct allocator_for_temp_list
{
    // forward list of free blocks:
    // free         | size | next | ..... |
    // allocated    | size | ...... align |
    //                      ^
    //                  aligned
    // size == total size of all block
    
    static const constexpr size_t total_size = 64 * 1024;
    static const constexpr size_t align_by = alignof(std::max_align_t);
    
    void * alloc (size_t size);
    void free (void * value) noexcept;
    
    static allocator_for_temp_list & get ()
    {
        static allocator_for_temp_list answer;
        return answer;
    }
    
    size_t available () const noexcept
    {
        size_t answer = 0;
        char * value = first_free;
        while (value)
        {
            answer += allocators::read_size(value);
            value = allocators::read_next(value);
        }
        return answer;
    }
    
private:
    allocator_for_temp_list () :
        data(std::make_unique_for_overwrite <char []> (align_by + total_size)),
        first_free(data.get() + align_by - sizeof(size_t))
    {
        size_t size = total_size;
        allocators::write_size(first_free, size);
        allocators::write_next(first_free, nullptr);
    }
    
    std::unique_ptr <char []> data;
    char * first_free;
};


struct allocator_for_temp_avl
{
    // forward list of free blocks:
    // free         | size | tree_data_t | ..... |
    // allocated    | size | ............. align |
    //                      ^
    //                  aligned
    // size == total size of all block
    
    static const constexpr size_t total_size = 64 * 1024;
    static const constexpr size_t align_by = alignof(std::max_align_t);
    
    void * alloc (size_t size);
    void free (void * value) noexcept;
    
    size_t available () const noexcept
    {
        return root? root->max_size : 0;
    }
    
    static allocator_for_temp_avl & get ()
    {
        static allocator_for_temp_avl answer;
        return answer;
    }
    
private:
    allocator_for_temp_avl () :
        data(std::make_unique_for_overwrite <char []> (align_by + total_size))
    {
        char * tree_place = data.get() + align_by - sizeof(size_t);
        size_t size = total_size;
        root = new (tree_place) tree_data_t(size);
    }
 
    struct tree_data_t
    {
        explicit tree_data_t () = default;
        
        tree_data_t (size_t _size) noexcept :
            left(nullptr),
            right(nullptr),
            parent(nullptr),
            size(_size),
            max_size(_size),
            rank(1)
        {}
        
        void update () noexcept
        {
            max_size = size;
            rank = 0;
            if (left)
            {
                max_size = std::max(max_size, left->max_size);
                rank = std::max(rank, left->rank);
            }
            if (right)
            {
                max_size = std::max(max_size, right->max_size);
                rank = std::max(rank, right->rank);
            }
            rank++;
        }
        
        void set_left (tree_data_t * new_left) noexcept
        {
            left = new_left;
            if (new_left)
                new_left->parent = this;
        }
        
        void set_right (tree_data_t * new_right) noexcept
        {
            right = new_right;
            if (new_right)
                new_right->parent = this;
        }
        
        tree_data_t * left;
        tree_data_t * right;
        tree_data_t * parent;
        size_t size;
        size_t max_size;
        uint32_t rank;
    };
    
    static void set_for_parent (tree_data_t * parent, tree_data_t * old_node, tree_data_t * new_node) noexcept
    {
        if (parent)
        {
            if (parent->left == old_node)
                parent->left = new_node;
            else if (parent->right == old_node)
                parent->right = new_node;
            else
                std::cerr << "unrechable in set_for_parent (" << parent << " -> " << parent->left << ", " << parent->right << "  vs " << old_node << std::endl;
        }
    }
    
    static void update_all (tree_data_t * value) noexcept
    {
        while (value)
        {
            value->update();
            value = value->parent;
        }
    }

    std::pair <tree_data_t *, tree_data_t *> find (void * value) const noexcept;
    tree_data_t * find (size_t need_size) const noexcept;
 
    static tree_data_t * merge_if_right_bigger (tree_data_t * left, tree_data_t * right) noexcept;
    static tree_data_t * merge_if_left_bigger (tree_data_t * left, tree_data_t * right) noexcept;
    static tree_data_t * merge (tree_data_t * left, tree_data_t * right) noexcept;
    
    static tree_data_t * balance (tree_data_t * cur) noexcept;
    
    std::unique_ptr <char []> data;
    tree_data_t * root;
};


struct allocator_for_temp_block
{
    // forward list of free blocks:
    // free         | next | ..... |
    // allocated    | ...... align |
    //               ^
    //            aligned

    static const constexpr size_t max_total_size = 64 * 1024;
    static const constexpr size_t max_count = 256;
 
    void * alloc (size_t size)
    {
        if (size > (1 << max_size))
            return operator new (size);
        size_t index = std::bit_width(size * 2 - 1);
        index = (index >= min_size)? index - min_size : 0;
        for (size_t i = index; i != data.size(); ++i)
        {
            if (first_block[i])
            {
                char * answer = first_block[i];
                first_block[i] = read_next(answer);
                return answer;
            }
        }
        return operator new (size);
    }
    
    void free (void * value) noexcept
    {
        if (!value)
            return;
        for (size_t i = 0; i != data.size(); ++i)
        {
            if (data[i].get() <= value && value <= data[i].get() + calc_size(i).needed_size)
            {
                char * cur = reinterpret_cast <char *> (value);
                write_next(cur, first_block[i]);
                first_block[i] = cur;
                return;
            }
        }
        operator delete (value);
    }
    
    static allocator_for_temp_block & get ()
    {
        static allocator_for_temp_block answer;
        return answer;
    }

    size_t available () const noexcept
    {
        size_t answer = 0;
        for (size_t i = 0; i != data.size(); ++i)
        {
            char * cur = first_block[i];
            size_t blocks = 0;
            while (cur)
            {
                blocks++;
                cur = read_next(cur);
            }
            answer += calc_size(i).block_size * blocks;
        }
        return answer;
    }
    
private:
    allocator_for_temp_block ()
    {
        for (size_t i = 0; i != data.size(); ++i)
        {
            calc_size sizes(i);
            data[i] = std::make_unique_for_overwrite <char []> (sizes.needed_size);
            first_block[i] = data[i].get();

            char * cur = first_block[i];
            for (size_t j = 0; j != sizes.count; ++j)
            {
                char * next = (j + 1 != sizes.count)? cur + sizes.block_size : nullptr;
                write_next(cur, next);
                cur = next;
            }
        }
    }
    
    struct calc_size
    {
        calc_size (size_t index)
        {
            block_size = 1 << (index + min_size);
            count = std::min(max_count, max_total_size / block_size);
            needed_size = block_size * count;
        }
        
        size_t block_size;
        size_t count;
        size_t needed_size;
    };

    static char * read_next (char * where) noexcept
    {
        char * answer;
        memcpy(&answer, where, sizeof(char *));
        return answer;
    }
    
    static void write_next (char * where, char * value) noexcept
    {
        memcpy(where, &value, sizeof(char *));
    }
    
    static const constexpr size_t min_size =  6; // from 64
    static const constexpr size_t max_size = 11; // to 2048

    std::array <std::unique_ptr <char []>, max_size - min_size + 1> data;
    std::array <char *, max_size - min_size + 1> first_block;
};

    
struct allocator_for_temp_block_2
{
    // forward list of free blocks:
    // free         | size | next | ..... |
    // allocated    | size | ...... align |
    //                      ^
    //                  aligned
    // size == total size of all block
    
    static const constexpr size_t max_total_size = 64 * 1024;
    static const constexpr size_t max_count = 256;
    static const constexpr size_t align_by = alignof(std::max_align_t);
 
    void * alloc (size_t size)
    {
        size_t index = std::bit_width(size * 2 - 1);
        index = (index >= min_size)? index - min_size : 0;
        for (size_t i = index; i < data.size(); ++i)
        {
            if (first_block[i]) [[likely]]
            {
                char * answer = first_block[i];
                first_block[i] = allocators::read_next(answer);
                return answer + sizeof(size_t);
            }
        }
        char * answer = static_cast <char *> (operator new (align_by + size));
        allocators::write_size(answer + align_by - sizeof(size_t), data.size());
        return answer + align_by;
    }
    
    void free (void * value) noexcept
    {
        if (!value)
            return;
        char * cur = static_cast <char *> (value) - sizeof(size_t);
        size_t index = allocators::read_size(cur);
        if (index >= data.size()) [[unlikely]]
        {
            operator delete (cur + sizeof(size_t) - align_by);
            return;
        }
        allocators::write_next(cur, first_block[index]);
        first_block[index] = cur;
    }
    
    static allocator_for_temp_block_2 & get ()
    {
        static allocator_for_temp_block_2 answer;
        return answer;
    }

    size_t available () const noexcept
    {
        size_t answer = 0;
        for (size_t i = 0; i != data.size(); ++i)
        {
            char * cur = first_block[i];
            size_t blocks = 0;
            while (cur)
            {
                blocks++;
                cur = allocators::read_next(cur);
            }
            answer += calc_size(i).block_size * blocks;
        }
        return answer;
    }
    
private:
    allocator_for_temp_block_2 ()
    {
        for (size_t i = 0; i != data.size(); ++i)
        {
            calc_size sizes(i);
            data[i] = std::make_unique_for_overwrite <char []> (sizes.needed_size);
            first_block[i] = data[i].get() + align_by - sizeof(size_t);

            char * cur = first_block[i];
            for (size_t j = 0; j != sizes.count; ++j)
            {
                char * next = (j + 1 != sizes.count)? cur + align_by + sizes.block_size : nullptr;
                allocators::write_size(cur, i);
                allocators::write_next(cur, next);
                cur = next;
            }
        }
    }
    
    struct calc_size
    {
        calc_size (size_t index)
        {
            block_size = 1 << (index + min_size);
            count = std::min(max_count, max_total_size / block_size);
            needed_size = align_by + (align_by + block_size) * count;
        }
        
        size_t block_size;
        size_t count;
        size_t needed_size;
    };

    static const constexpr size_t min_size =  6; // from 64
    static const constexpr size_t max_size = 11; // to 2048

    std::array <std::unique_ptr <char []>, max_size - min_size + 1> data;
    std::array <char *, max_size - min_size + 1> first_block;
};


template <typename T, std::enable_if_t <alignof(T) <= alignof(std::max_align_t), int> = 0>
struct allocator_for_temp
{
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    
    using allocator_type = allocator_for_temp_block_2;
    
    template <typename V>
    struct rebind
    {
        using other = allocator_for_temp <V>;
    };
    
    [[nodiscard]] constexpr T * allocate (size_t n)
    {
        return static_cast <T *> (allocator_type::get().alloc(sizeof(T) * n));
    }
    
    void deallocate (T * p, size_t)
    {
        allocator_type::get().free(p);
    }
};


#endif

