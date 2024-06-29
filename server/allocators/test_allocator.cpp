#include "allocator.h"
#include <iomanip>
#include <array>
#include <iostream>
#include <vector>
#include <algorithm>

struct allocator_for_temp_standart
{
    void * alloc (size_t size)
    {
        return operator new (size);
    }
    
    void free (void * value) noexcept
    {
        operator delete (value);
    }
    
    static allocator_for_temp_standart & get ()
    {
        static allocator_for_temp_standart answer;
        return answer;
    }

    size_t available () const noexcept
    {
        return 0;
    }
    
private:
    allocator_for_temp_standart () = default;
};


int main ()
{
    // using allocator = allocator_for_temp_block_2;
    // using allocator = allocator_for_temp_block;
    // using allocator = allocator_for_temp_avl;
    // using allocator = allocator_for_temp_list;
    using allocator = allocator_for_temp_standart;
    
    auto modules = std::to_array <size_t>
    ({
        101, 103, 107, 109, 113, 119, 123, 127, 131, 137, 139
    });
    
    auto sizes = std::to_array <size_t>
    ({
        64, 128, 256, 512, 1024
    });
    
    std::vector <size_t> counts;
    for (size_t count = 2; count != *std::max_element(modules.begin(), modules.end()); ++count)
    {
        bool prime = 1;
        for (size_t div : counts)
        {
            if (count % div == 0)
            {
                prime = 0;
                break;
            }
            if (div * div >= count)
                break;
        }
        if (prime)
            counts.push_back(count);
    }
    
    size_t available = allocator::get().available();
    uint32_t iterations = 0;
    
    for (size_t repeats = 0; repeats != 100; ++repeats)
    // for (size_t repeats = 0; repeats != 3000; ++repeats)
    {
        for (size_t module : modules)
        {
            for (size_t count : counts)
            {
                std::vector <bool> visit(count, 0);
                std::vector <void *> ptr(count, nullptr);
                
                size_t value = 1;
                size_t max_allocated = 0;
                size_t allocated = 0;
                for (size_t i = 0; i != 4 * module; ++i)
                {
                    iterations++;
                    size_t index = value % count;
                    visit[index] = 1;
                    if (ptr[index])
                    {
                        // std::cout << "free " << ptr[index] << " ";
                        allocator::get().free(ptr[index]);
                        ptr[index] = nullptr;
                        allocated--;
                    }
                    else
                    {
                        size_t alloc_size = sizes[value % sizes.size()];
                        ptr[index] = allocator::get().alloc(alloc_size);
                        if (repeats == 0)
                            memset(ptr[index], 0xaa, alloc_size);
                        allocated++;
                        max_allocated = std::max(max_allocated, allocated);
                        // std::cout << "alloc " << ptr[index] << " ";
                    }
                    // std::cout << allocator::get().available() << std::endl;
                    value = (value * 2) % module;
                }
                
                size_t visited = 0;
                for (size_t index = 0; index != count; ++index)
                {
                    visited += visit[index];
                    if (ptr[index])
                        allocator::get().free(ptr[index]);
                    ptr[index] = nullptr;
                }
                
                if (repeats == 0)
                {
                    if (allocator::get().available() != available)
                        std::cout << "!!!not all freed!!! (" << allocator::get().available() << ")" << std::endl;
                    std::cout << std::setw(3) << module << " % " << std::setw(3) << count << " -> " << std::setw(3) << max_allocated << " / " << std::setw(3) << visited << std::endl;
                }
            }
            if (repeats == 0)
                std::cout << std::endl;
        }
    }
    std::cout << iterations << " iterations" << std::endl;
}


