#include "response_catapult.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"
#include "base_comparators.h"
#include "append_row.h"


struct catapult_cmp
{
    static registrator_cmp <catapult::catapult_partial> sort;
    static registrator_cmp <catapult::catapult_partial> group;
    static registrator_pred <catapult::catapult_partial> & filter ();
};

registrator_cmp <catapult::catapult_partial> catapult_cmp::sort
({
    { "name_ru",        comparators::name_ru <catapult::catapult_partial> },
    { "name_en",        comparators::name_en <catapult::catapult_partial> },
    { "class",          comparators::classes <catapult::catapult_partial> },
    { "in_service",     comparators::in_service <catapult::catapult_partial> },
    { "acceleration",   comparators::universal <catapult::catapult_partial, double, &catapult::catapult_partial::acceleration> },
    { "speed",          comparators::universal <catapult::catapult_partial, double, &catapult::catapult_partial::speed> },
    { "launch_mass",    comparators::universal <catapult::catapult_partial, double, &catapult::catapult_partial::launch_mass> },
});

registrator_cmp <catapult::catapult_partial> catapult_cmp::group
({
    { "class",      comparators::classes <catapult::catapult_partial> },
    { "in_service", comparators::in_service_10th <catapult::catapult_partial> },
});

registrator_pred <catapult::catapult_partial> & catapult_cmp::filter ()
{
    static registrator_pred <catapult::catapult_partial> answer;
    if (answer.empty())
    {
        answer.reg <year_filter  <catapult::catapult_partial> > ("in_service");
        answer.reg <class_filter <catapult::catapult_partial> > ("class");
        answer.reg <id_filter    <catapult::catapult_partial> > ("id");
    }
    
    return answer;
}


void catapult::response (simple_string & answer, std::string_view query)
{
    std::vector <std::vector <catapult_partial> > list_group = 
         parse_group_and_sort <catapult_partial, catapult_cmp> (catapult_cache, query);

    for (std::vector <catapult_partial> const & list : list_group)
    {
        answer.append(table::begin);
        
        append_row <catapult_text, &catapult_text::name> (answer, list, text_cache);
        
        answer.append("длина");
        append_row <catapult_text, &catapult_text::length> (answer, list, text_cache);
        
        answer.append("ширина");
        append_row <catapult_text, &catapult_text::width> (answer, list, text_cache);
        
        answer.append("скорость");
        append_row <catapult_text, &catapult_text::speed> (answer, list, text_cache);
        
        answer.append("запускаемая масса");
        append_row <catapult_text, &catapult_text::launch_mass> (answer, list, text_cache);
        
        answer.append("ускорение");
        append_row <catapult_text, &catapult_text::alleceration> (answer, list, text_cache);
        
        answer.append("на вооружении");
        for (catapult_partial const & item : list)
            answer.append(text_cache[item.index].in_service);
        
        answer.append(table::end);
        answer.append("<br>");
    }
}


catapult::catapult_partial::catapult_partial (catapult_t const & value, size_t _index) :
    index(_index),
    id          (value.id),
    class_id    (value.class_id),
    name_ru     (value.catapult_ru),
    name_en     (value.catapult_en),
    acceleration(value.alleceration .value_or(std::numeric_limits <double> ::infinity())),
    speed       (value.speed        .value_or(std::numeric_limits <double> ::infinity())),
    launch_mass (value.launch_mass  .value_or(std::numeric_limits <double> ::infinity())),
    in_service  (value.in_service)
{}

catapult::catapult_text::catapult_text (catapult_t const & item) :
    name        (table::new_column),
    length      (table::new_column),
    width       (table::new_column),
    speed       (table::new_column),
    launch_mass (table::new_column),
    alleceration(table::new_column),
    in_service  (table::new_column)
{
    name.append(item.class_ru.value_or(" "))
        .append(table::new_line)
        .append(item.catapult_ru.value_or(" "));
    length      .append(item.length? to_string_10(*item.length) + "м" : " ");
    width       .append(item.width? to_string_10(*item.width) + "м" : " ");
    speed       .append(item.speed? to_string_10(*item.speed) + "м/с" : " ");
    launch_mass .append(item.launch_mass? to_string_10(*item.launch_mass) + "кг" : " ");
    alleceration.append(item.alleceration? to_string_10(*item.alleceration) + "g" : " ");
    in_service  .append(item.in_service? to_string(*item.in_service) : " ");
}


