#ifndef GROUP_AND_SORTING_H
#define GROUP_AND_SORTING_H

#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include <map>
#include <chrono>
#include <span>
#include "parse_query.h"


template <typename T>
struct comparator
{
    comparator
    (
        std::function <std::partial_ordering (T const &, T const &)> _cmp, 
        std::unique_ptr <comparator <T> > _next
    ) :
        cmp(std::move(_cmp)),
        next(std::move(_next))
    {}
    
    std::partial_ordering compare
    (
        T const & a, 
        T const & b
    )
    {
        std::partial_ordering res = cmp(a, b);
        if (res != std::partial_ordering::equivalent &&
            res != std::partial_ordering::unordered)
            return res;
        if (next)
            return next->compare(a, b);
        else
            return std::partial_ordering::equivalent;
    }
 
private:
    std::function <std::partial_ordering (T const &, T const &)> cmp;
    std::unique_ptr <comparator <T> > next;
};


template <typename T>
struct comparator_for_sort
{
    comparator_for_sort (comparator <T> * _cmp) :
        cmp(std::move(_cmp))
    {}
    
    bool operator () (T const & a, T const & b)
    {
        std::partial_ordering ans = cmp? cmp->compare(a, b) : std::partial_ordering::equivalent;
        return std::is_lt(ans);
    }

private:
    comparator <T> * cmp;
};


template <typename T>
struct filter
{
    filter 
    (
        std::function <bool (T const &)> _predicate,
        std::unique_ptr <filter <T> > _next
    ) :
        predicate(std::move(_predicate)),
        next(std::move(_next))
    {}
    
    bool operator () (T const & value)
    {
        return predicate(value) && 
               (!next || next->operator () (value));
    }

private:
    std::function <bool (T const &)> predicate;
    std::unique_ptr <filter <T> > next;
};


template <typename T>
struct filter_for_sort
{
    filter_for_sort (filter <T> * _predicate) :
        predicate(std::move(_predicate))
    {}
    
    bool operator () (T const & value)
    {
        return predicate? predicate->operator () (value) : 1;
    }

private:
    filter <T> * predicate;
};


template <typename T>
std::vector <std::vector <T> > 
group_and_sort 
(
    std::vector <T> const & values_unfiltered,
    filter_for_sort <T> predicate,
    comparator_for_sort <T> group_cmp,
    comparator_for_sort <T> sort_cmp
)
{
    if (values_unfiltered.empty())
        return {};

    std::vector <T> values;
    for (T const & value : values_unfiltered)
        if (predicate(value))
            values.push_back(value);
    
    std::vector <std::vector <T> > answer;

    std::sort(values.begin(), values.end(), group_cmp);
    
    bool group_start = 1;
    for (size_t i = 0; i != values.size(); )
    {
        if (group_start)
        {
            group_start = 0;
            answer.push_back({std::move(values[i++])});
            continue;
        }
        
        if (group_cmp(answer.back()[0], values[i]))
        {
            group_start = 1;
            continue;
        }
        
        answer.back().push_back(std::move(values[i++]));
    }
    
    for (std::vector <T> & group : answer)
        std::sort(group.begin(), group.end(), sort_cmp);

    return answer;
}


template <typename T, typename cmp_location>
std::vector <std::vector <T> >
parse_group_and_sort
(
    std::vector <T> const & torpedoes,
    std::string_view query
)
{
    std::vector <group> parsed = parse_query(query);
    
    std::unique_ptr <filter <T> > predicate;
    std::unique_ptr <comparator <T> > group_cmp;
    std::unique_ptr <comparator <T> > sort_cmp;
    
    for (group const & cur_gr : parsed)
    {
        if (cur_gr.key == "group")
            group_cmp = cmp_location::group.get(cur_gr.values, std::move(group_cmp));
        if (cur_gr.key == "sort")
            sort_cmp = cmp_location::sort.get(cur_gr.values, std::move(sort_cmp));
        if (cur_gr.key == "filter")
            predicate = cmp_location::filter().get(cur_gr.values, std::move(predicate));
    }

    return group_and_sort
    (
        torpedoes, 
        filter_for_sort <T> (predicate.get()),
        comparator_for_sort <T> (group_cmp.get()), 
        comparator_for_sort <T> (sort_cmp.get())
    );
}


#endif

