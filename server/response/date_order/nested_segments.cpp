#include "nested_segments.h"


#include <algorithm>
#include "dekart_tree.h"

template <typename T>
std::strong_ordering compare_null
(
    std::optional <T> const & a, std::optional <T> const & b, 
    std::strong_ordering not_a, std::strong_ordering not_b
)
{
    if (!a && !b)
        return std::strong_ordering::equal;
    if (a && b)
        return *a <=> *b;
    if (!a)
        return not_a;
    if (!b)
        return not_b;
    return std::strong_ordering::equal; // just for aviod warning...
}

template <typename T>
std::strong_ordering compare_null_first (std::optional <T> const & a, std::optional <T> const & b)
{
    return compare_null(a, b, std::strong_ordering::less, std::strong_ordering::greater);
}

template <typename T>
std::strong_ordering compare_null_last (std::optional <T> const & a, std::optional <T> const & b)
{
    return compare_null(a, b, std::strong_ordering::greater, std::strong_ordering::less);
}

// for sort
auto comparator_bEi = [] (segment const & a, segment const & b) -> bool 
{ 
    std::strong_ordering begin = compare_null_first(a.begin, b.begin);
    if (begin != std::strong_ordering::equal)
        return is_lt(begin);
    
    std::strong_ordering end = compare_null_last(a.end, b.end);
    if (end != std::strong_ordering::equal) // longest should be first
        return is_gt(end);
    
    return a.index < b.index;
};

// for tree
auto comparator_ei = [] (segment const & a, segment const & b) -> bool
{
    std::strong_ordering end = compare_null_last(a.end, b.end);
    if (end != std::strong_ordering::equal)
        return is_lt(end);
    
    return a.index < b.index;
};

// for accumulate from tree
auto calc_min_Bei = [] (segment const & a, segment const & b) -> segment
{
    std::strong_ordering begin = compare_null_first(a.begin, b.begin);
    if (begin != std::strong_ordering::equal)
        return is_lt(begin)? b : a;
    
    std::strong_ordering end = compare_null_last(a.end, b.end);
    if (end != std::strong_ordering::equal)
        return is_lt(end)? a : b;
    
    return (a.index < b.index)? a : b;
};


std::vector <std::vector <size_t> >
nested_segments 
(std::vector <segment> values)
{
    std::vector <std::vector <size_t> > answer(values.size());
    
    std::sort(values.begin(), values.end(), comparator_bEi);
    
    
    typedef dekart_tree <segment, segment, decltype(comparator_ei)> dekart_tree_seg;
    
    auto updater = [] (dekart_tree_seg::node const * vertex) -> segment
    {
        segment answer = vertex->key;
        if (vertex->left)
            answer = calc_min_Bei(answer, vertex->left->additional);
        if (vertex->right)
            answer = calc_min_Bei(answer, vertex->right->additional);
        return answer;
    };
    
    auto accumulator_left = [] (segment value, dekart_tree_seg::node const * vertex) -> segment
    {
        value = calc_min_Bei(value, vertex->key);
        if (vertex->right)
            value = calc_min_Bei(value, vertex->right->additional);
        return value;
    };

    auto accumulator_right = [] (segment value, dekart_tree_seg::node const *) -> segment
    {
        return value;
    };
    
    dekart_tree_seg tree(updater);

    segment max_Bei = {std::nullopt, std::nullopt, std::numeric_limits <size_t> ::max()};
    
    for (size_t i = 0; i != values.size(); )
    {
        size_t j;
        for (j = i; j != values.size(); ++j)
        {
            if (values[i].begin != values[j].begin ||
                values[i].end   != values[j].end)
                break;
            segment find_key = values[j];
            find_key.index = 0;
            segment upper = tree.accumulate(find_key, max_Bei, accumulator_left, accumulator_right);
            if (upper.index != max_Bei.index)
                answer[upper.index].push_back(values[j].index);
        }
        
        for (size_t k = i; k != j; ++k)
            tree.insert(values[k], values[k]);

        i = j;
    }
    
    return answer;
}

