#ifndef REGISTRATORS_H
#define REGISTRATORS_H

#include "group_and_sorting.h"
#include <chrono>
#include <functional>
#include <optional>


template <typename T>
struct registrator_cmp
{
    typedef std::function <std::partial_ordering (T const &, T const &)> func_t;
    
    registrator_cmp (std::vector <std::pair <std::string, func_t> > func_list)
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


template <typename T>
struct registrator_pred
{
    typedef std::function <bool (T const &)> func_t;
    
    template <typename U>
    void reg (std::string name)
    {
        functions.insert({std::move(name), std::make_unique <predicate_creator <U> > ()});
    }
    
    std::unique_ptr <filter <T> > 
    get 
    (
        std::vector <std::string_view> const & names,
        std::unique_ptr <filter <T> > predicate
    )
    {
        if (names.empty())
            return predicate;
        typename std::map <std::string, creator_t> :: iterator it = functions.find(std::string(names[0]));
        if (it == functions.end())
            return predicate;
        return std::make_unique <filter <T> > 
               (
                    it->second->create(std::span <std::string_view const> (names.begin() + 1, names.end())), 
                    std::move(predicate)
               );
    }

    struct predicate_creator_base
    {
        virtual ~predicate_creator_base () = default;
        virtual func_t create (std::span <std::string_view const> ) = 0;
    };
    
    template <typename U>
    struct predicate_creator : predicate_creator_base
    {
        virtual func_t create (std::span <std::string_view const> values)
        {
            return U(values);
        }
        
        virtual ~predicate_creator () = default;
    };

    bool empty ()
    {
        return functions.empty();
    }
    
private:
    typedef std::unique_ptr <predicate_creator_base> creator_t;
    std::map <std::string, creator_t> functions;
};


#endif

