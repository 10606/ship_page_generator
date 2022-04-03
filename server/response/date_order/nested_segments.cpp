#include "nested_segments.h"


#include <algorithm>
#include "dekart_tree.h"


template <typename T>
std::strong_ordering compare_null_first (std::optional <T> const & a, std::optional <T> const & b)
{
    if (!a && !b)
        return std::strong_ordering::equal;
    if (a && b)
        return *a <=> *b;
    if (!a)
        return std::strong_ordering::less;
    if (!b)
        return std::strong_ordering::greater;
}

// for sort
auto comparator_bEi = [] (segment const & a, segment const & b) -> bool 
{ 
    if (a.begin < b.begin)
        return 1;
    if (b.begin < a.begin)
        return 0;
    
    if (a.end < b.end) // longest should be first
        return 0;
    if (b.end < a.end)
        return 1;
    
    return a.index < b.index;
};

// for tree
auto comparator_ei = [] (segment const & a, segment const & b) -> bool
{
    if (a.end < b.end)
        return 1;
    if (b.end < a.end)
        return 0;
    
    return a.index < b.index;
};

// for accumulate from tree
auto calc_min_Bei = [] (segment const & a, segment const & b) -> segment
{
    if (a.begin < b.begin)
        return b;
    if (b.begin < a.begin)
        return a;
    
    if (a.end < b.end)
        return a;
    if (b.end < a.end)
        return b;
    
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

    std::chrono::year_month_day ymd_min(std::chrono::year::min(), std::chrono::month(1),  std::chrono::day(1));
    std::chrono::year_month_day ymd_max(std::chrono::year::max(), std::chrono::month(12), std::chrono::day(31));
    segment max_Bei = {ymd_min, ymd_max, std::numeric_limits <size_t> ::max()};
    
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

