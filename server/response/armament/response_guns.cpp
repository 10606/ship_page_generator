#include "response_guns.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"
#include "base_comparators.h"


struct guns_cmp
{
    static registrator_cmp <guns::guns_t> sort;
    static registrator_cmp <guns::guns_t> group;
    static registrator_pred <guns::guns_t> & filter ();
};

registrator_cmp <guns::guns_t> guns_cmp::sort
({
    {
        "caliber", comparators::caliber <guns::guns_t>
    },
    {
        "in_service", comparators::in_service <guns::guns_t>
    },
    {
        "name_ru", 
        [] (guns::guns_t const & a, guns::guns_t const & b)
            { return compare_null_last(a.gun_ru, b.gun_ru); }
    },
    {
        "name_en", 
        [] (guns::guns_t const & a, guns::guns_t const & b)
            { return compare_null_last(a.gun_en, b.gun_en); }
    },
    {
        "class", comparators::in_service <guns::guns_t>
    },
});

registrator_cmp <guns::guns_t> guns_cmp::group
({
    {
        "caliber", comparators::caliber <guns::guns_t>
    },
    {
        "in_service", comparators::in_service <guns::guns_t>
    },
    {
        "class", comparators::in_service <guns::guns_t>
    },
});

registrator_pred <guns::guns_t> & guns_cmp::filter ()
{
    static registrator_pred <guns::guns_t> answer;
    if (answer.empty())
    {
        answer.reg <year_filter     <guns::guns_t> > ("in_service");
        answer.reg <caliber_filter  <guns::guns_t> > ("caliber");
        answer.reg <class_filter    <guns::guns_t> > ("class");
        answer.reg <id_filter       <guns::guns_t> > ("id");
    }
    
    return answer;
}


void guns::response (std::string & answer, std::string_view query)
{
    answer.reserve(10000);
    
    static std::vector <guns_t> guns_cache = database->armament_info.get_list();
    
    std::vector <std::vector <guns_t> > list_group = 
         parse_group_and_sort <guns_t, guns_cmp> (guns_cache, query);

    for (std::vector <guns_t> const & list : list_group)
    {
        answer.append(table::begin);
        
        for (guns_t const & item : list)
            answer.append(table::new_column)
                  .append(item.class_ru.value_or(" "))
                  .append(table::new_line)
                  .append(item.gun_ru.value_or(" "));
        answer.append(table::new_row);
        
        answer.append("калибр");
        for (guns_t const & item : list)
            answer.append(table::new_column)
                  .append(item.caliber? to_string_10(*item.caliber) + "мм" : " ");
        answer.append(table::new_row);
        
        answer.append("длина ствола");
        for (guns_t const & item : list)
            answer.append(table::new_column)
                  .append(item.length? to_string_10(*item.length) + "калибров" : " ");
        answer.append(table::new_row);
        
        answer.append("скорострельность");
        for (guns_t const & item : list)
            answer.append(table::new_column)
                  .append(item.rate_of_fire? to_string_10(*item.rate_of_fire) + "выстр/мин" : " ");
        answer.append(table::new_row);
        
        answer.append("эффективная дальность");
        for (guns_t const & item : list)
            answer.append(table::new_column)
                  .append(item.effective_range? to_string_10(*item.effective_range) + "м" : " ");
        answer.append(table::new_row);
        
        answer.append("масса");
        for (guns_t const & item : list)
            answer.append(table::new_column)
                  .append(item.mass? to_string_10(*item.mass) + "кг" : " ");
        answer.append(table::new_row);
        
        answer.append("построено");
        for (guns_t const & item : list)
            answer.append(table::new_column)
                  .append(item.build_cnt? to_string_10(*item.build_cnt) + "шт" : " ");
        answer.append(table::new_row);
        
        answer.append("на вооружении");
        for (guns_t const & item : list)
            answer.append(table::new_column)
                  .append(item.in_service? to_string(*item.in_service) : " ");
        
        answer.append(table::end);
    }
}


