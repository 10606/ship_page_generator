#include "response_searchers.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"


struct searcher_cmp
{
    static registrator_cmp <searcher::searcher_t> sort;
    static registrator_cmp <searcher::searcher_t> group;
    static registrator_pred <searcher::searcher_t> & filter ();
};

registrator_cmp <searcher::searcher_t> searcher_cmp::sort
({
    {
        "in_service", 
        [] (searcher::searcher_t const & a, searcher::searcher_t const & b)
            { return compare_null_last(a.in_service, b.in_service); }
    },
    {
        "mass", 
        [] (searcher::searcher_t const & a, searcher::searcher_t const & b)
            { return compare_null_last(a.mass, b.mass); }
    },
    {
        "power", 
        [] (searcher::searcher_t const & a, searcher::searcher_t const & b)
            { return compare_null_last(a.power, b.power); }
    },
    {
        "name_ru", 
        [] (searcher::searcher_t const & a, searcher::searcher_t const & b)
            { return compare_null_last(a.searcher_ru, b.searcher_ru); }
    },
    {
        "name_en", 
        [] (searcher::searcher_t const & a, searcher::searcher_t const & b)
            { return compare_null_last(a.searcher_en, b.searcher_en); }
    },
    {
        "class", 
        [] (searcher::searcher_t const & a, searcher::searcher_t const & b)
            { return a.class_id <=> b.class_id; }
    },
});

registrator_cmp <searcher::searcher_t> searcher_cmp::group
({
    {
        "in_service", 
        [] (searcher::searcher_t const & a, searcher::searcher_t const & b)
            { return compare_date_10th(a.in_service, b.in_service); }
    },
    {
        "class", 
        [] (searcher::searcher_t const & a, searcher::searcher_t const & b)
            { return a.class_id <=> b.class_id; }
    },
});

registrator_pred <searcher::searcher_t> & searcher_cmp::filter ()
{
    static registrator_pred <searcher::searcher_t> answer;
    if (answer.empty())
    {
        answer.reg <year_filter  <searcher::searcher_t> > ("in_service");
        answer.reg <class_filter <searcher::searcher_t> > ("class");
        answer.reg <id_filter    <searcher::searcher_t> > ("id");
    }
    
    return answer;
}


std::string searcher::response (std::string_view query)
{
    std::string answer;
    
    std::vector <std::vector <searcher_t> > list_group = 
         parse_group_and_sort <searcher_t, searcher_cmp> (database->armament_info.get_searchers(), query);

    for (std::vector <searcher_t> const & list : list_group)
    {
        answer.append(table::begin);
        
        for (searcher_t const & item : list)
            answer.append(table::new_column)
                  .append(item.class_ru.value_or(" "))
                  .append(table::new_line)
                  .append(item.searcher_ru.value_or(" "));
        answer.append(table::new_row);
        
        answer.append("масса");
        for (searcher_t const & item : list)
            answer.append(table::new_column)
                  .append(item.mass? to_string_10(*item.mass) + "кг" : " ");
        answer.append(table::new_row);
        
        answer.append("частота");
        for (searcher_t const & item : list)
            answer.append(table::new_column)
                  .append(item.frequency? to_string_10(*item.frequency) + "МГц" : " ");
        answer.append(table::new_row);
        
        answer.append("мощность");
        for (searcher_t const & item : list)
            answer.append(table::new_column)
                  .append(item.power? to_string_10(*item.power) + "кВт" : " ");
        answer.append(table::new_row);
        
        answer.append("построено");
        for (searcher_t const & item : list)
            answer.append(table::new_column)
                  .append(item.build_cnt? to_string_10(*item.build_cnt) + "шт" : " ");
        answer.append(table::new_row);
        
        answer.append("на вооружении");
        for (searcher_t const & item : list)
            answer.append(table::new_column)
                  .append(item.in_service? to_string(*item.in_service) : " ");
        
        answer.append(table::end);
    }

    return answer;
}


