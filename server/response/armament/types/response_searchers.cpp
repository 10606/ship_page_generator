#include "response_searchers.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"
#include "base_comparators.h"


struct searcher_cmp
{
    static registrator_cmp <searcher::searchers_partial> sort;
    static registrator_cmp <searcher::searchers_partial> group;
    static registrator_pred <searcher::searchers_partial> & filter ();
};

registrator_cmp <searcher::searchers_partial> searcher_cmp::sort
({
    {
        "in_service", comparators::in_service <searcher::searchers_partial>
    },
    {
        "mass", 
        [] (searcher::searchers_partial const & a, searcher::searchers_partial const & b)
            { return a.mass <=> b.mass; }
    },
    {
        "power", 
        [] (searcher::searchers_partial const & a, searcher::searchers_partial const & b)
            { return a.power <=> b.power; }
    },
    {
        "name_ru", comparators::name_ru <searcher::searchers_partial>
    },
    {
        "name_en", comparators::name_en <searcher::searchers_partial>
    },
    {
        "class", comparators::classes <searcher::searchers_partial>
    },
});

registrator_cmp <searcher::searchers_partial> searcher_cmp::group
({
    {
        "in_service", comparators::in_service_10th <searcher::searchers_partial>
    },
    {
        "class", comparators::classes <searcher::searchers_partial>
    },
});

registrator_pred <searcher::searchers_partial> & searcher_cmp::filter ()
{
    static registrator_pred <searcher::searchers_partial> answer;
    if (answer.empty())
    {
        answer.reg <year_filter  <searcher::searchers_partial> > ("in_service");
        answer.reg <class_filter <searcher::searchers_partial> > ("class");
        answer.reg <id_filter    <searcher::searchers_partial> > ("id");
    }
    
    return answer;
}


void searcher::response (simple_string & answer, std::string_view query)
{
    std::vector <std::vector <searchers_partial> > list_group = 
         parse_group_and_sort <searchers_partial, searcher_cmp> (searchers_cache, query);

    for (std::vector <searchers_partial> const & list : list_group)
    {
        answer.append(table::begin);
        
        for (searchers_partial const & item : list)
            answer.append(text_cache[item.index].name);
        answer.append(table::new_row);
        
        answer.append("масса");
        for (searchers_partial const & item : list)
            answer.append(text_cache[item.index].mass);
        answer.append(table::new_row);
        
        answer.append("частота");
        for (searchers_partial const & item : list)
            answer.append(text_cache[item.index].frequency);
        answer.append(table::new_row);
        
        answer.append("мощность");
        for (searchers_partial const & item : list)
            answer.append(text_cache[item.index].power);
        answer.append(table::new_row);
        
        answer.append("построено");
        for (searchers_partial const & item : list)
            answer.append(text_cache[item.index].build_cnt);
        answer.append(table::new_row);
        
        answer.append("на вооружении");
        for (searchers_partial const & item : list)
            answer.append(text_cache[item.index].in_service);
        
        answer.append(table::end);
    }
}


searcher::searchers_partial::searchers_partial (searcher_t const & value, size_t _index) :
    index(_index),
    id          (value.id),
    class_id    (value.class_id),
    name_ru     (value.searcher_ru),
    name_en     (value.searcher_en),
    mass        (value.mass .value_or(std::numeric_limits <double> ::infinity())),
    power       (value.power.value_or(std::numeric_limits <double> ::infinity())),
    in_service  (value.in_service)
{}

searcher::searchers_text::searchers_text (searcher_t const & item) :
    name        (table::new_column),
    mass        (table::new_column),
    frequency   (table::new_column),
    power       (table::new_column),
    build_cnt   (table::new_column),
    in_service  (table::new_column)
{
    name      .append(item.searcher_ru.value_or(" "));
    mass      .append(item.mass? to_string_10(*item.mass) + "кг" : " ");
    frequency .append(item.frequency? to_string_10(*item.frequency) + "МГц" : " ");
    power     .append(item.power? to_string_10(*item.power) + "кВт" : " ");
    build_cnt .append(item.build_cnt? to_string_10(*item.build_cnt) + "шт" : " ");
    in_service.append(item.in_service? to_string(*item.in_service) : " ");
}


