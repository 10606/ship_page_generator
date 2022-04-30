#include "response_torpedo.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"


struct torpedo_cmp
{
    static registrator_cmp <torpedo::torpedo_t> sort;
    static registrator_cmp <torpedo::torpedo_t> group;
    static registrator_pred <torpedo::torpedo_t> & filter ();
};

registrator_cmp <torpedo::torpedo_t> torpedo_cmp::sort
({
    {
        "caliber", 
        [] (torpedo::torpedo_t const & a, torpedo::torpedo_t const & b)
            { return compare_null_last(a.caliber, b.caliber); }
    },
    {
        "mass_ex", 
        [] (torpedo::torpedo_t const & a, torpedo::torpedo_t const & b)
            { return compare_null_last(a.mass_ex, b.mass_ex); }
    },
    {
        "in_service", 
        [] (torpedo::torpedo_t const & a, torpedo::torpedo_t const & b)
            { return compare_null_last(a.in_service, b.in_service); }
    },
    {
        "name_ru", 
        [] (torpedo::torpedo_t const & a, torpedo::torpedo_t const & b)
            { return compare_null_last(a.torpedo_ru, b.torpedo_ru); }
    },
    {
        "name_en", 
        [] (torpedo::torpedo_t const & a, torpedo::torpedo_t const & b)
            { return compare_null_last(a.torpedo_en, b.torpedo_en); }
    },
});

registrator_cmp <torpedo::torpedo_t> torpedo_cmp::group
({
    {
        "caliber", 
        [] (torpedo::torpedo_t const & a, torpedo::torpedo_t const & b)
            { return compare_null_last(a.caliber, b.caliber); }
    },
    {
        "in_service", 
        [] (torpedo::torpedo_t const & a, torpedo::torpedo_t const & b)
            { return compare_date_10th(a.in_service, b.in_service); }
    },
});

registrator_pred <torpedo::torpedo_t> & torpedo_cmp::filter ()
{
    static registrator_pred <torpedo::torpedo_t> answer;
    if (answer.empty())
    {
        answer.reg <year_filter     <torpedo::torpedo_t> > ("in_service");
        answer.reg <caliber_filter  <torpedo::torpedo_t> > ("caliber");
        answer.reg <id_filter       <torpedo::torpedo_t> > ("id");
    }
    
    return answer;
}


void torpedo::response (std::string & answer, std::string_view query)
{
    answer.reserve(10000);
    
    static std::vector <torpedo_t> torpedo_cache = database->armament_info.get_torpedo();

    std::vector <std::vector <torpedo_t> > list_group = 
         parse_group_and_sort <torpedo_t, torpedo_cmp> (torpedo_cache, query);

    for (std::vector <torpedo_t> const & list : list_group)
    {
        answer.append(table::begin);
        
        for (torpedo_t const & item : list)
            answer.append(table::new_column)
                  .append(item.torpedo_ru.value_or(" "));
        answer.append(table::new_row);
        
        answer.append("калибр");
        for (torpedo_t const & item : list)
            answer.append(table::new_column)
                  .append(item.caliber? to_string_10(*item.caliber) + "мм" : " ");
        answer.append(table::new_row);
        
        answer.append("длина");
        for (torpedo_t const & item : list)
            answer.append(table::new_column)
                  .append(item.length? to_string_10(*item.length) + "мм" : " ");
        answer.append(table::new_row);
        
        answer.append("скорость");
        for (torpedo_t const & item : list)
            answer.append(table::new_column)
                  .append(item.speed? to_string_10(*item.speed) + "узлов" : " ");
        answer.append(table::new_row);
        
        answer.append("дальность");
        for (torpedo_t const & item : list)
            answer.append(table::new_column)
                  .append(item.range? to_string_10(*item.range) + "м" : " ");
        answer.append(table::new_row);
        
        answer.append("масса");
        for (torpedo_t const & item : list)
            answer.append(table::new_column)
                  .append(item.mass? to_string_10(*item.mass) + "кг" : " ");
        answer.append(table::new_row);
        
        answer.append("масса ВВ");
        for (torpedo_t const & item : list)
            answer.append(table::new_column)
                  .append(item.mass_ex? to_string_10(*item.mass_ex) + "кг" : " ");
        answer.append(table::new_row);
        
        answer.append("на вооружении");
        for (torpedo_t const & item : list)
            answer.append(table::new_column)
                  .append(item.in_service? to_string(*item.in_service) : " ");
        
        answer.append(table::end);
    }
}


