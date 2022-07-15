#include "response_torpedo_tubes.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"
#include "base_comparators.h"


struct torpedo_tubes_cmp
{
    static registrator_cmp <torpedo_tubes::torpedo_tubes_partial> sort;
    static registrator_cmp <torpedo_tubes::torpedo_tubes_partial> group;
    static registrator_pred <torpedo_tubes::torpedo_tubes_partial> & filter ();
};

registrator_cmp <torpedo_tubes::torpedo_tubes_partial> torpedo_tubes_cmp::sort
({
    {
        "caliber", comparators::caliber <torpedo_tubes::torpedo_tubes_partial>
    },
    {
        "in_service", comparators::in_service <torpedo_tubes::torpedo_tubes_partial>
    },
    {
        "name_ru", comparators::name_ru <torpedo_tubes::torpedo_tubes_partial>
    },
    {
        "name_en", comparators::name_en <torpedo_tubes::torpedo_tubes_partial>
    },
    {
        "class", comparators::classes <torpedo_tubes::torpedo_tubes_partial>
    },
});

registrator_cmp <torpedo_tubes::torpedo_tubes_partial> torpedo_tubes_cmp::group
({
    {
        "caliber", comparators::caliber <torpedo_tubes::torpedo_tubes_partial>
    },
    {
        "in_service", comparators::in_service_10th <torpedo_tubes::torpedo_tubes_partial>
    },
    {
        "class", comparators::classes <torpedo_tubes::torpedo_tubes_partial>
    },
});

registrator_pred <torpedo_tubes::torpedo_tubes_partial> & torpedo_tubes_cmp::filter ()
{
    static registrator_pred <torpedo_tubes::torpedo_tubes_partial> answer;
    if (answer.empty())
    {
        answer.reg <year_filter     <torpedo_tubes::torpedo_tubes_partial> > ("in_service");
        answer.reg <caliber_filter  <torpedo_tubes::torpedo_tubes_partial> > ("caliber");
        answer.reg <class_filter    <torpedo_tubes::torpedo_tubes_partial> > ("class");
        answer.reg <id_filter       <torpedo_tubes::torpedo_tubes_partial> > ("id");
    }
    
    return answer;
}


void torpedo_tubes::response (simple_string & answer, std::string_view query)
{
    std::vector <std::vector <torpedo_tubes_partial> > list_group = 
         parse_group_and_sort <torpedo_tubes_partial, torpedo_tubes_cmp> (torpedo_tubes_cache, query);

    for (std::vector <torpedo_tubes_partial> const & list : list_group)
    {
        answer.append(table::begin);
        
        for (torpedo_tubes_partial const & item : list)
            answer.append(text_cache[item.index].name);
        answer.append(table::new_row);
        
        answer.append("калибр");
        for (torpedo_tubes_partial const & item : list)
            answer.append(text_cache[item.index].caliber);
        answer.append(table::new_row);
        
        answer.append("количество труб");
        for (torpedo_tubes_partial const & item : list)
            answer.append(text_cache[item.index].tubes_count);
        answer.append(table::new_row);
        
        answer.append("на вооружении");
        for (torpedo_tubes_partial const & item : list)
            answer.append(text_cache[item.index].in_service);
        
        answer.append(table::end);
        answer.append("<br>");
    }
}


torpedo_tubes::torpedo_tubes_partial::torpedo_tubes_partial (torpedo_tubes_t const & value, size_t _index) :
    index(_index),
    id          (value.id),
    class_id    (value.class_id),
    name_ru     (value.tube_ru),
    name_en     (value.tube_en),
    caliber     (value.caliber.value_or(std::numeric_limits <double> ::infinity())),
    in_service  (value.in_service)
{}

torpedo_tubes::torpedo_tubes_text::torpedo_tubes_text (torpedo_tubes_t const & item) :
    name        (table::new_column),
    caliber     (table::new_column),
    tubes_count (table::new_column),
    in_service  (table::new_column)
{
    name.append(item.class_ru.value_or(" "))
        .append(table::new_line)
        .append(item.tube_ru.value_or(" "));
    caliber    .append(item.caliber? to_string_10(*item.caliber) + "мм" : " ");
    tubes_count.append(item.tubes_count? std::to_string(*item.tubes_count) : " ");
    in_service .append(item.in_service? to_string(*item.in_service) : " ");
}


