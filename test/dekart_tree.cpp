#include "dekart_tree.h"

#include <gtest/gtest.h>

typedef dekart_tree <int, int> tree_t;
auto sum_calc = [] (dekart_tree <int, int> ::node const * tree) -> int 
{ 
    int ans = tree->key;
    if (tree->left)
        ans += tree->left->additional;
    if (tree->right)
        ans += tree->right->additional;
    return ans;
};

TEST(insert, simple)
{
    dekart_tree <int, int> tree(sum_calc);
    
    EXPECT_NO_THROW(tree.insert( 1,  1));
    EXPECT_NO_THROW(tree.insert( 5,  5));
    EXPECT_NO_THROW(tree.insert( 3,  3));
    EXPECT_NO_THROW(tree.insert(-2, -2));
    EXPECT_NO_THROW(tree.insert( 2,  2));
    
    auto check = [] (int, dekart_tree <int, int> ::node const * tree) -> int
    {
        if (tree->left && tree->left->key > tree->key)
            throw std::runtime_error("order left");
        if (tree->right && tree->right->key < tree->key)
            throw std::runtime_error("order right");
        return 0;
    };
    
    EXPECT_NO_THROW(tree.accumulate(0, 0, check, check));

    EXPECT_EQ(tree.vertex().second, 9);
}

TEST(sum, ge_1)
{
    dekart_tree <int, int> tree(sum_calc);
    
    EXPECT_NO_THROW(tree.insert( 1,  1));
    EXPECT_NO_THROW(tree.insert( 5,  5));
    EXPECT_NO_THROW(tree.insert( 3,  3));
    EXPECT_NO_THROW(tree.insert(-2, -2));
    EXPECT_NO_THROW(tree.insert( 2,  2));
    
    // sum >=
    auto sum = [] (int value, dekart_tree <int, int> ::node const * tree) -> int
    {
        int ans = value + tree->key;
        if (tree->right)
            ans += tree->right->additional;
        return ans;
    };
    
    auto id = [] (int value, dekart_tree <int, int> ::node const *) -> int
    {
        return value;
    };
    
    EXPECT_EQ(tree.accumulate(-3, 0, sum, id),  9);
    EXPECT_EQ(tree.accumulate( 0, 0, sum, id), 11);
    EXPECT_EQ(tree.accumulate( 2, 0, sum, id), 10);
    EXPECT_EQ(tree.accumulate( 3, 0, sum, id),  8);
    EXPECT_EQ(tree.accumulate( 4, 0, sum, id),  5);
    EXPECT_EQ(tree.accumulate( 5, 0, sum, id),  5);
    EXPECT_EQ(tree.accumulate( 6, 0, sum, id),  0);
}

TEST(sum, ge_2)
{
    dekart_tree <int, int> tree(sum_calc);
    
    EXPECT_NO_THROW(tree.insert(-2, -2));
    EXPECT_NO_THROW(tree.insert( 3,  3));
    EXPECT_NO_THROW(tree.insert( 5,  5));
    EXPECT_NO_THROW(tree.insert( 1,  1));
    EXPECT_NO_THROW(tree.insert( 2,  2));
    
    // sum >=
    auto sum = [] (int value, dekart_tree <int, int> ::node const * tree) -> int
    {
        int ans = value + tree->key;
        if (tree->right)
            ans += tree->right->additional;
        return ans;
    };
    
    auto id = [] (int value, dekart_tree <int, int> ::node const *) -> int
    {
        return value;
    };
    
    EXPECT_EQ(tree.accumulate(-3, 0, sum, id),  9);
    EXPECT_EQ(tree.accumulate( 0, 0, sum, id), 11);
    EXPECT_EQ(tree.accumulate( 2, 0, sum, id), 10);
    EXPECT_EQ(tree.accumulate( 3, 0, sum, id),  8);
    EXPECT_EQ(tree.accumulate( 4, 0, sum, id),  5);
    EXPECT_EQ(tree.accumulate( 5, 0, sum, id),  5);
    EXPECT_EQ(tree.accumulate( 6, 0, sum, id),  0);
}

