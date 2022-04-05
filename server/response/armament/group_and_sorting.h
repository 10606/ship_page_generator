#ifndef GROUP_AND_SORTING_H
#define GROUP_AND_SORTING_H

#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include <map>
#include <chrono>


template <typename T>
std::partial_ordering 
compare_null_last 
(
    std::optional <T> const & a, 
    std::optional <T> const & b
)
{
    if (!a && !b)
        return std::partial_ordering::equivalent;
    if (a && b)
        return *a <=> *b;
    if (!a)
        return std::partial_ordering::greater;
    if (!b)
        return std::partial_ordering::less;
    return std::partial_ordering::equivalent; // just for aviod warning
}


std::partial_ordering 
compare_date_10th 
(
    std::optional <std::chrono::year_month_day> const & a, 
    std::optional <std::chrono::year_month_day> const & b
)
{
    if (!a && !b)
        return std::partial_ordering::equivalent;
    if (a && b)
        return (static_cast <int> (a->year()) / 10) <=> (static_cast <int> (b->year()) / 10);
    if (!a)
        return std::partial_ordering::greater;
    if (!b)
        return std::partial_ordering::less;
    return std::partial_ordering::equivalent; // just for aviod warning
}


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
std::vector <std::vector <T> > 
group_and_sort 
(
    std::vector <T> && values,
    comparator_for_sort <T> group_cmp,
    comparator_for_sort <T> sort_cmp
)
{
    if (values.empty())
        return {};
    
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


template <typename T>
struct registrator
{
    typedef std::function <std::partial_ordering (T const &, T const &)> func_t;
    
    registrator (std::vector <std::pair <std::string, func_t> > func_list)
    {
        for (auto & func : func_list)
            functions.insert(std::move(func));
    }
    
    void reg (std::string name, func_t func)
    {
        functions.insert({std::move(name), std::move(func)});
    }
    
    std::unique_ptr <comparator <T> > 
    get 
    (
        std::string_view name, 
        std::unique_ptr <comparator <T> > cmp
    )
    {
        typename std::map <std::string, func_t> :: iterator it = functions.find(std::string(name));
        if (it == functions.end())
            return cmp;
        return std::make_unique <comparator <T> > (it->second, std::move(cmp));
    }
    
    std::unique_ptr <comparator <T> > 
    get
    (
        std::vector <std::string_view> const & names,
        std::unique_ptr <comparator <T> > answer
    )
    {
        for (size_t i = names.size(); i--; )
            answer = get(names[i], std::move(answer));
        return answer;
    }
       
private:
    std::map <std::string, func_t> functions;
};


#endif

