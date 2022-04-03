#ifndef DEKART_TREE_H
#define DEKART_TREE_H

#include <chrono>
#include <concepts>
#include <functional>
#include <memory>
#include <random>

template <typename F, typename U, typename node>
concept Accumulator = requires (F && f, U acc, node const * tree)
{
    {f(acc, tree)} -> std::convertible_to <U>;
};

template <typename T, typename U, std::predicate <T, T> C = std::less <T>>
struct dekart_tree
{
    struct node
    {
        std::unique_ptr <node> left;
        std::unique_ptr <node> right;
        
        T key;
        U additional;
        size_t priority;
    };
    
    typedef std::unique_ptr <node> ptr_type;
    
    dekart_tree 
    (
        std::function <U (node const *)> _updater
    ) :
        root(),
        updater(std::move(_updater)),
        comparator()
    {}
        
    void insert (T key, U additional);
    
    std::pair <T, U> vertex ()
    {
        if (!root)
            return {T(), U()};
        return {root->key, root->additional};
    };

    template <typename V, Accumulator <V, node> F_left, Accumulator <V, node> F_right>
    V accumulate
    (
        T key, V init,
        F_left && accumulator_left,
        F_right && accumulator_right
    ) const
    {
        return accumulate(root.get(), std::move(key), std::move(init), std::forward <F_left> (accumulator_left), std::forward <F_right> (accumulator_right));
    }
    
private:
    ptr_type root;
    std::function <U (node const *)> updater;
    C comparator;
    
    //  <   >=
    std::pair <ptr_type, ptr_type>
    split (ptr_type tree, T const & key);

    template <typename V, Accumulator <V, node> F_left, Accumulator <V, node> F_right>
    V accumulate
    (
        node const * tree, 
        T key, 
        V init,
        F_left && accumulator_left,
        F_right && accumulator_right
    ) const;

    ptr_type insert (ptr_type tree, T key, U additional, size_t priority);
};



template <typename T, typename U, std::predicate <T, T> C>
std::pair <typename dekart_tree <T, U, C> ::ptr_type, typename dekart_tree <T, U, C> ::ptr_type>
dekart_tree <T, U, C> ::split 
(typename dekart_tree <T, U, C> ::ptr_type tree, T const & key)
{
    if (!tree)
        return {nullptr, nullptr};
  
    if (comparator(tree->key, key)) // go to right
    {
        std::pair <ptr_type, ptr_type> splited = split(std::move(tree->right), key);
        tree->right = std::move(splited.first);
        tree->additional = updater(tree.get());
        return {std::move(tree), std::move(splited.second)};
    } 
    else // go to left
    {
        std::pair <ptr_type, ptr_type> splited = split(std::move(tree->left), key);
        tree->left = std::move(splited.second);
        tree->additional = updater(tree.get());
        return {std::move(splited.first), std::move(tree)};
    }
}


template <typename T, typename U, std::predicate <T, T> C>
typename dekart_tree <T, U, C> ::ptr_type 
dekart_tree <T, U, C> ::insert
(typename dekart_tree <T, U, C> ::ptr_type tree, T key, U additional, size_t priority)
{
    if (!tree || tree->priority < priority)
    {
        std::pair <ptr_type, ptr_type> splited = split(std::move(tree), key);
        ptr_type answer = std::make_unique <node> 
            (std::move(splited.first), std::move(splited.second), std::move(key), std::move(additional), priority);
        answer->additional = updater(answer.get());
        return answer;
    }
    
    if (comparator(tree->key, key)) // go to right
        tree->right = insert(std::move(tree->right), std::move(key), std::move(additional), priority);
    else // go to left
        tree->left = insert(std::move(tree->left), std::move(key), std::move(additional), priority);
    tree->additional = updater(tree.get());
    return tree;
}


template <typename T, typename U, std::predicate <T, T> C>
template <typename V,
          Accumulator <V, typename dekart_tree <T, U, C> ::node> F_left, 
          Accumulator <V, typename dekart_tree <T, U, C> ::node> F_right>
V
dekart_tree <T, U, C> ::accumulate
(
    typename dekart_tree <T, U, C> ::node const * tree,
    T key, 
    V init,
    F_left && accumulator_left,
    F_right && accumulator_right
) const
{
    while (tree)
    {
        if (comparator(tree->key, key)) // go to right
        {
            init = accumulator_right(std::move(init), tree);
            tree = tree->right.get();
        }
        else // go to left
        {
            init = accumulator_left(std::move(init), tree);
            tree = tree->left.get();
        }
    }
    return std::move(init);
}


struct random_generator
{
    random_generator () :
        rd(),
        gen(rd()),
        distr(0)
    {}
    
    size_t operator () ()
    {
        return distr(gen);
    }

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution <size_t> distr;
};


template <typename T, typename U, std::predicate <T, T> C>
void dekart_tree <T, U, C> ::insert (T key, U additional)
{
    static random_generator randomizer;
    root = insert(std::move(root), std::move(key), std::move(additional), randomizer());
}


#endif

