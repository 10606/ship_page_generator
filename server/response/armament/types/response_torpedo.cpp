#include "response_torpedo.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"
#include "base_comparators.h"


struct torpedo_cmp
{
    static registrator_cmp <torpedo::torpedo_partial> sort;
    static registrator_cmp <torpedo::torpedo_partial> group;
    static registrator_pred <torpedo::torpedo_partial> & filter ();
};

registrator_cmp <torpedo::torpedo_partial> torpedo_cmp::sort
({
    {
        "caliber", comparators::caliber <torpedo::torpedo_partial>
    },
    {
        "mass_ex", 
        [] (torpedo::torpedo_partial const & a, torpedo::torpedo_partial const & b)
            { return a.mass_ex <=> b.mass_ex; }
    },
    {
        "in_service", comparators::in_service <torpedo::torpedo_partial>
    },
    {
        "name_ru", comparators::name_ru <torpedo::torpedo_partial>
    },
    {
        "name_en", comparators::name_en <torpedo::torpedo_partial>
    },
});

registrator_cmp <torpedo::torpedo_partial> torpedo_cmp::group
({
    {
        "caliber", comparators::caliber <torpedo::torpedo_partial>
    },
    {
        "in_service", comparators::in_service_10th <torpedo::torpedo_partial>
    },
});

registrator_pred <torpedo::torpedo_partial> & torpedo_cmp::filter ()
{
    static registrator_pred <torpedo::torpedo_partial> answer;
    if (answer.empty())
    {
        answer.reg <year_filter     <torpedo::torpedo_partial> > ("in_service");
        answer.reg <caliber_filter  <torpedo::torpedo_partial> > ("caliber");
        answer.reg <id_filter       <torpedo::torpedo_partial> > ("id");
    }
    
    return answer;
}


void torpedo::response (std::string & answer, std::string_view query)
{
    answer.reserve(10000);
    
    std::vector <std::vector <torpedo_partial> > list_group = 
         parse_group_and_sort <torpedo_partial, torpedo_cmp> (torpedo_cache, query);

    for (std::vector <torpedo_partial> const & list : list_group)
    {
        answer.append(table::begin);
        
        for (torpedo_partial const & item : list)
            answer.append(text_cache[item.index].name);
        answer.append(table::new_row);
        
        answer.append("калибр");
        for (torpedo_partial const & item : list)
            answer.append(text_cache[item.index].caliber);
        answer.append(table::new_row);
        
        answer.append("длина");
        for (torpedo_partial const & item : list)
            answer.append(text_cache[item.index].length);
        answer.append(table::new_row);
        
        answer.append("скорость");
        for (torpedo_partial const & item : list)
            answer.append(text_cache[item.index].speed);
        answer.append(table::new_row);
        
        answer.append("дальность");
        for (torpedo_partial const & item : list)
            answer.append(text_cache[item.index].range);
        answer.append(table::new_row);
        
        answer.append("масса");
        for (torpedo_partial const & item : list)
            answer.append(text_cache[item.index].mass);
        answer.append(table::new_row);
        
        answer.append("масса ВВ");
        for (torpedo_partial const & item : list)
            answer.append(text_cache[item.index].mass_ex);
        answer.append(table::new_row);
        
        answer.append("на вооружении");
        for (torpedo_partial const & item : list)
            answer.append(text_cache[item.index].in_service);
        
        answer.append(table::end);
    }
}


torpedo::torpedo_partial::torpedo_partial (torpedo_t const & value, size_t _index) :
    index(_index),
    id          (value.id),
    name_ru     (value.torpedo_ru),
    name_en     (value.torpedo_en),
    caliber     (value.caliber.value_or(std::numeric_limits <double> ::infinity())),
    mass_ex     (value.mass_ex.value_or(std::numeric_limits <double> ::infinity())),
    in_service  (value.in_service)
{}

torpedo::torpedo_text::torpedo_text (torpedo_t const & item) :
    name        (table::new_column),
    caliber     (table::new_column),
    length      (table::new_column),
    speed       (table::new_column),
    range       (table::new_column),
    mass        (table::new_column),
    mass_ex     (table::new_column),
    in_service  (table::new_column)
{
    name      .append(item.torpedo_ru.value_or(" "));
    caliber   .append(item.caliber? to_string_10(*item.caliber) + "мм" : " ");
    length    .append(item.length? to_string_10(*item.length) + "мм" : " ");
    speed     .append(item.speed? to_string_10(*item.speed) + "узлов" : " ");
    range     .append(item.range? to_string_10(*item.range) + "м" : " ");
    mass      .append(item.mass? to_string_10(*item.mass) + "кг" : " ");
    mass_ex   .append(item.mass_ex? to_string_10(*item.mass_ex) + "кг" : " ");
    in_service.append(item.in_service? to_string(*item.in_service) : " ");
}


