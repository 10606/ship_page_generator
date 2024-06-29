#include "allocator.h"

void * allocator_for_temp_list::alloc (size_t size)
{
    size_t needed_size = (size + sizeof(size_t) + align_by - 1) & ~(align_by - 1);
    
    char ** for_rewrite = &first_free;
    char * cur = first_free;
    while (cur)
    {
        size_t cur_size = allocators::read_size(cur);
        char * next;
        memcpy(&next, cur + sizeof(size_t), sizeof(char *));
        
        if (cur_size >= needed_size + 
                        align_by + sizeof(size_t) + sizeof(char *))
        {
            // allocate with split
            size_t tail_size = cur_size - needed_size;
            allocators::write_size(cur, needed_size);
            
            allocators::write_size(cur + needed_size, tail_size);
            allocators::write_next(cur + needed_size, next);
            
            *for_rewrite = cur + needed_size;
            
            return cur + sizeof(size_t);
        }
        if (cur_size >= needed_size)
        {
            // allocate all block
            *for_rewrite = next;
            return cur + sizeof(size_t);
        }
        {
            // not enough -> skip
            for_rewrite = reinterpret_cast <char **> (cur + sizeof(size_t));
            cur = next;
        }
    }
    
    return operator new (size);
}

void allocator_for_temp_list::free (void * value) noexcept
{
    if (!value)
        return;
    char * end = data.get() + align_by + total_size;
    if (value < data.get() || value >= end)
    {
        operator delete (value);
        return;
    }
    
    char * cur = static_cast <char *> (value) - sizeof(size_t);
    size_t cur_size = allocators::read_size(cur);
    
    char * prev = nullptr;
    char * next = first_free;
    while (next)
    {
        if (next > cur)
            break;
        prev = next;
        next = allocators::read_next(next);
    }
    
    allocators::write_next(cur, next);
    if (prev)
    {
        size_t prev_size = allocators::read_size(prev);
        if (prev + prev_size != cur)
            allocators::write_next(prev, cur);
        else
        {
            // merge
            prev_size += cur_size;
            allocators::write_size(prev, prev_size);
            allocators::write_next(prev, next);
            
            cur = prev;
            cur_size = prev_size;
        }
    }
    else
    {
        first_free = cur;
    }
        
    if (next && cur + cur_size == next)
    {
        // merge
        size_t next_size = allocators::read_size(next);
        char * next_next = allocators::read_next(next);
        cur_size += next_size;
        allocators::write_size(cur, cur_size);
        allocators::write_next(cur, next_next);
    }
}

