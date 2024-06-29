#include "allocator.h"

void * allocator_for_temp_avl::alloc (size_t size)
{
    // std::cerr << "alloc" << std::endl;
    size_t needed_size = (size + sizeof(size_t) + align_by - 1) & ~(align_by - 1);
    
    tree_data_t * node = find(needed_size);
    if (!node)
        return operator new (size);
    
    static const constexpr size_t min_size = std::max(sizeof(tree_data_t), sizeof(size_t) + align_by * 2);
    tree_data_t * parent = node->parent;
    tree_data_t * new_node;
    bool get_all_node = node->size < needed_size + min_size;
    if (!get_all_node) [[likely]]
    {
        char * new_place = reinterpret_cast <char *> (node) + needed_size;
        new_node = new (new_place) tree_data_t(*node);
        new_node->size -= needed_size;
        if (new_node->left)
            new_node->left->parent = new_node;
        if (new_node->right)
            new_node->right->parent = new_node;
    }
    else
    {
        needed_size = node->size;
        new_node = merge(node->left, node->right);
    }
    
    if (parent)
        set_for_parent(parent, node, new_node);
    else
        root = new_node;
    
    if (get_all_node)
    {
        if (new_node)
        {
            new_node->parent = parent;
            new_node->update();
            root = balance(new_node);
        }
        else
            root = balance(parent);
    }
    else [[likely]]
        update_all(new_node);

    char * answer = reinterpret_cast <char *> (node);
    node->~tree_data_t();
    new (answer) size_t(needed_size);
    return answer + sizeof(size_t);
}

void allocator_for_temp_avl::free (void * value) noexcept
{
    // std::cerr << "free " << value << std::endl;
    if (!value)
        return;
    char * end = data.get() + align_by + total_size;
    if (value < data.get() || value >= end)
    {
        operator delete (value);
        return;
    }
    
    char * cur = reinterpret_cast <char *> (value) - sizeof(size_t);
    size_t cur_size;
    memcpy(&cur_size, cur, sizeof(size_t));
    std::pair <tree_data_t *, tree_data_t *> nieghbour = find(cur);
    char * left  = reinterpret_cast <char *> (nieghbour.first);
    char * right = reinterpret_cast <char *> (nieghbour.second);
    
    bool close_with_left = nieghbour.first && left + nieghbour.first->size == cur;
    bool close_with_right = nieghbour.second && cur + cur_size == right;
    if (close_with_left && !close_with_right)
    {
        // std::cerr << "free (close with left)" << " " << nieghbour.first << " " << nieghbour.second << std::endl;
        nieghbour.first->size += cur_size;
        update_all(nieghbour.first);
    }
    if (close_with_right && !close_with_left)
    {
        // std::cerr << "free (close with right)" << " " << nieghbour.first << " " << nieghbour.second << std::endl;
        tree_data_t * new_node = new (cur) tree_data_t(*nieghbour.second);
        new_node->size += cur_size;
        if (nieghbour.second->parent)
            set_for_parent(nieghbour.second->parent, nieghbour.second, new_node);
        else
            root = new_node;
        if (new_node->left)
            new_node->left->parent = new_node;
        if (new_node->right)
            new_node->right->parent = new_node;
        nieghbour.second->~tree_data_t();
        update_all(new_node);
    }
    if (!close_with_left && !close_with_right)
    {
        // std::cerr << "free (not close with any)" << " " << nieghbour.first << " " << nieghbour.second << std::endl;
        tree_data_t * new_node = new (cur) tree_data_t(cur_size);
        if (nieghbour.first && !nieghbour.first->right)
        {
            nieghbour.first->right = new_node;
            new_node->parent = nieghbour.first;
            root = balance(nieghbour.first);
        }
        else if (nieghbour.second && !nieghbour.second->left)
        {
            nieghbour.second->left = new_node;
            new_node->parent = nieghbour.second;
            root = balance(nieghbour.second);
        }
        else if (!nieghbour.first && !nieghbour.second)
        {
            root = new_node;
        }
        else
        {
            std::cerr << "unrechable in free (not close with any)" << std::endl;
        }
    }
    if (close_with_left && close_with_right)
    {
        if (!nieghbour.first->right)
        {
            // std::cerr << "free (close with both, left down)" << " " << nieghbour.first << " " << nieghbour.second << std::endl;
            tree_data_t * left_left   = nieghbour.first->left;
            tree_data_t * left_parent = nieghbour.first->parent; // exist
            size_t left_size = nieghbour.first->size;
            *nieghbour.first = tree_data_t(*nieghbour.second);
            nieghbour.first->size += left_size + cur_size;
            if (left_parent != nieghbour.second && nieghbour.second->left)
                nieghbour.second->left->parent = nieghbour.first;
            if (nieghbour.second->right)
                nieghbour.second->right->parent = nieghbour.first;
            set_for_parent(nieghbour.second->parent, nieghbour.second, nieghbour.first);
            
            if (left_parent == nieghbour.second)
            {
                nieghbour.first->left = left_left;
                left_parent = nieghbour.first;
            }
            else
            {
                set_for_parent(left_parent, nieghbour.first, left_left);
            }
            if (left_left)
                left_left->parent = left_parent;
            nieghbour.second->~tree_data_t();
            root = balance(left_parent);
        }
        else if (!nieghbour.second->left)
        {
            // std::cerr << "free (close with both, right down)" << nieghbour.first << " " << nieghbour.second << std::endl;
            tree_data_t * right_right  = nieghbour.second->right;
            tree_data_t * right_parent = nieghbour.second->parent; // exist
            nieghbour.first->size += cur_size + nieghbour.second->size;
            
            if (right_parent == nieghbour.first)
                nieghbour.first->right = right_right;
            else
                set_for_parent(right_parent, nieghbour.second, right_right);
            if (right_right)
                right_right->parent = right_parent;
            nieghbour.second->~tree_data_t();
            root = balance(right_parent);
        }
        else
        {
            std::cerr << "unrechable in free (close with both)" << std::endl;
        }
    }
}

std::pair <allocator_for_temp_avl::tree_data_t *, allocator_for_temp_avl::tree_data_t *> allocator_for_temp_avl::find (void * value) const noexcept
{
    tree_data_t * before_last_go_right = nullptr;
    tree_data_t * before_last_go_left = nullptr;
    tree_data_t * cur = root;
    
    if (!cur)
        return {nullptr, nullptr};
    while (1)
    {
        if (cur > value)
        {
            // go left
            if (!cur->left)
                return {before_last_go_right, cur};
            before_last_go_left = cur;
            cur = cur->left;
        }
        else
        {
            // go right
            if (!cur->right)
                return {cur, before_last_go_left};
            before_last_go_right = cur;
            cur = cur->right;
        }
    }
}

allocator_for_temp_avl::tree_data_t * allocator_for_temp_avl::find (size_t need_size) const noexcept
{
    tree_data_t * cur = root;
    while (cur)
    {
        if (cur->size >= need_size)
            return cur;
        if (cur->max_size < need_size)
            return nullptr;
        if (cur->left && cur->left->max_size >= need_size)
            cur = cur->left;
        else
            cur = cur->right;
    }
    return nullptr;
}

allocator_for_temp_avl::tree_data_t * allocator_for_temp_avl::merge_if_right_bigger (tree_data_t * left, tree_data_t * right) noexcept
{
    tree_data_t * prev_removed = nullptr;
    tree_data_t * removed = left;
    while (removed->right)
    {
        prev_removed = removed;
        removed = removed->right;
    }
    if (prev_removed)
    {
        prev_removed->right = removed->left;
        if (removed->left)
            removed->left->parent = prev_removed;
    }
    else
        left = left->left;
    
    uint32_t left_rank = left? left->rank : 0;
    tree_data_t * merge_to = right;
    tree_data_t * prev_merge_to = nullptr;
    while (merge_to && left_rank + 1 < merge_to->rank)
    {
        prev_merge_to = merge_to;
        merge_to = merge_to->left;
    }
    removed->left = left;
    removed->right = merge_to;
    removed->parent = prev_merge_to;
    if (left)
        left->parent = removed;
    if (merge_to)
        merge_to->parent = removed;
    if (prev_merge_to)
        prev_merge_to->left = removed;
    if (prev_removed)
        return balance(prev_removed);
    else
        return balance(removed);
}

allocator_for_temp_avl::tree_data_t * allocator_for_temp_avl::merge_if_left_bigger (tree_data_t * left, tree_data_t * right) noexcept
{
    tree_data_t * prev_removed = nullptr;
    tree_data_t * removed = right;
    while (removed->left)
    {
        prev_removed = removed;
        removed = removed->left;
    }
    if (prev_removed)
    {
        prev_removed->left = removed->right;
        if (removed->right)
            removed->right->parent = prev_removed;
    }
    else
        right = right->right;
    
    uint32_t right_rank = right? right->rank : 0;
    tree_data_t * merge_to = left;
    tree_data_t * prev_merge_to = nullptr;
    while (merge_to && right_rank + 1 < merge_to->rank)
    {
        prev_merge_to = merge_to;
        merge_to = merge_to->right;
    }
    removed->left = merge_to;
    removed->right = right;
    removed->parent = prev_merge_to;
    if (merge_to)
        merge_to->parent = removed;
    if (right)
        right->parent = removed;
    if (prev_merge_to)
        prev_merge_to->right = removed;
    if (prev_removed)
        return balance(prev_removed);
    else
        return balance(removed);
}

allocator_for_temp_avl::tree_data_t * allocator_for_temp_avl::merge (tree_data_t * left, tree_data_t * right) noexcept
{
    // std::cerr << "merge" << std::endl;
    if (!left)
        return right;
    if (!right)
        return left;
    left->parent = nullptr;
    right->parent = nullptr;
    if (left->rank >= right->rank)
        return merge_if_left_bigger(left, right);
    else
        return merge_if_right_bigger(left, right);
}

allocator_for_temp_avl::tree_data_t * allocator_for_temp_avl::balance (tree_data_t * cur) noexcept
{
    while (cur)
    {
        tree_data_t * left = cur->left;
        tree_data_t * right = cur->right;
        uint32_t left_rank  = left?  left->rank  : 0;
        uint32_t right_rank = right? right->rank : 0;
        if (left_rank + 1 >= right_rank && left_rank <= right_rank + 1)
        {
            cur->update(); // cur - not updated parent in previous
            if (!cur->parent)
                return cur;
            cur = cur->parent;
            continue;
        }
        tree_data_t * parent = cur->parent;
        tree_data_t * new_cur;
        if (left_rank + 2 == right_rank)
        {
            tree_data_t * right_right = right->right;
            uint32_t right_right_rank = right_right? right_right->rank : 0;
            if (right_right_rank + 1 == right_rank)
            {
                // rotate edge |cur - right| anti-clockwise
                // std::cerr << "balance rotate edge |cur - right| anti-clockwise" << std::endl;
                cur->set_right(right->left);
                cur->parent = right;
                
                right->left = cur;
                
                cur->update();
                right->update();
                new_cur = right;
            }
            else
            {
                // right left go up
                // std::cerr << "balance right left go up" << std::endl;
                tree_data_t * right_left = right->left;
                cur->set_right(right_left->left);
                cur->parent = right_left;
                
                right->set_left(right_left->right);
                right->parent = right_left;
                
                right_left->left = cur;
                right_left->right = right;
                
                cur->update();
                right->update();
                right_left->update();
                new_cur = right_left;
            }
        }
        else if (left_rank == right_rank + 2)
        {
            tree_data_t * left_left = left->left;
            uint32_t left_left_rank = left_left? left_left->rank : 0;
            if (left_left_rank + 1 == left_rank)
            {
                // rotate edge |cur - left| clockwise
                // std::cerr << "balance rotate edge |cur - left| clockwise" << std::endl;
                cur->set_left(left->right);
                cur->parent = left;
                
                left->right = cur;
                
                cur->update();
                left->update();
                new_cur = left;
            }
            else
            {
                // left right go up
                // std::cerr << "balance left right go up" << std::endl;
                tree_data_t * left_right = left->right;
                cur->set_left(left_right->right);
                cur->parent = left_right;
                
                left->set_right(left_right->left);
                left->parent = left_right;
                
                left_right->left = left;
                left_right->right = cur;
                
                cur->update();
                left->update();
                left_right->update();
                new_cur = left_right;
            }
        }
        else
        {
            std::cerr << "unrechable in balance " << left_rank << " " << right_rank << std::endl;
        }
        
        new_cur->parent = parent;
        set_for_parent(parent, cur, new_cur);
        // don't update parent twice
        if (!parent)
            return new_cur;
        cur = parent;
        continue;
    }
    return nullptr;
}
