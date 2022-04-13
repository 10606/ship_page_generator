#include "response_torpedo_tubes.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"


struct torpedo_tubes_cmp
{
    static registrator_cmp <torpedo_tubes::torpedo_tubes_t> sort;
    static registrator_cmp <torpedo_tubes::torpedo_tubes_t> group;
    static registrator_pred <torpedo_tubes::torpedo_tubes_t> & filter ();
};

registrator_cmp <torpedo_tubes::torpedo_tubes_t> torpedo_tubes_cmp::sort
({
    {
        "caliber", 
        [] (torpedo_tubes::torpedo_tubes_t const & a, torpedo_tubes::torpedo_tubes_t const & b)
            { return compare_null_last(a.caliber, b.caliber); }
    },
    {
        "in_service", 
        [] (torpedo_tubes::torpedo_tubes_t const & a, torpedo_tubes::torpedo_tubes_t const & b)
            { return compare_null_last(a.in_service, b.in_service); }
    },
    {
        "name_ru", 
        [] (torpedo_tubes::torpedo_tubes_t const & a, torpedo_tubes::torpedo_tubes_t const & b)
            { return compare_null_last(a.tube_ru, b.tube_ru); }
    },
    {
        "name_en", 
        [] (torpedo_tubes::torpedo_tubes_t const & a, torpedo_tubes::torpedo_tubes_t const & b)
            { return compare_null_last(a.tube_en, b.tube_en); }
    },
    {
        "class", 
        [] (torpedo_tubes::torpedo_tubes_t const & a, torpedo_tubes::torpedo_tubes_t const & b)
            { return a.class_id <=> b.class_id; }
    },
});

registrator_cmp <torpedo_tubes::torpedo_tubes_t> torpedo_tubes_cmp::group
({
    {
        "caliber", 
        [] (torpedo_tubes::torpedo_tubes_t const & a, torpedo_tubes::torpedo_tubes_t const & b)
            { return compare_null_last(a.caliber, b.caliber); }
    },
    {
        "in_service", 
        [] (torpedo_tubes::torpedo_tubes_t const & a, torpedo_tubes::torpedo_tubes_t const & b)
            { return compare_date_10th(a.in_service, b.in_service); }
    },
    {
        "class", 
        [] (torpedo_tubes::torpedo_tubes_t const & a, torpedo_tubes::torpedo_tubes_t const & b)
            { return a.class_id <=> b.class_id; }
    },
});

registrator_pred <torpedo_tubes::torpedo_tubes_t> & torpedo_tubes_cmp::filter ()
{
    static registrator_pred <torpedo_tubes::torpedo_tubes_t> answer;
    if (answer.empty())
    {
        answer.reg <year_filter     <torpedo_tubes::torpedo_tubes_t> > ("in_service");
        answer.reg <caliber_filter  <torpedo_tubes::torpedo_tubes_t> > ("caliber");
        answer.reg <class_filter    <torpedo_tubes::torpedo_tubes_t> > ("class");
        answer.reg <id_filter       <torpedo_tubes::torpedo_tubes_t> > ("id");
    }
    
    return answer;
}


std::string torpedo_tubes::response (std::string_view query)
{
    std::string answer;
    answer.reserve(10000);
    
    static std::vector <torpedo_tubes_t> torpedo_tubes_cache = database->armament_info.get_torpedo_tubes();
    
    std::vector <std::vector <torpedo_tubes_t> > list_group = 
         parse_group_and_sort <torpedo_tubes_t, torpedo_tubes_cmp> (torpedo_tubes_cache, query);

    for (std::vector <torpedo_tubes_t> const & list : list_group)
    {
        answer.append(table::begin);
        
        for (torpedo_tubes_t const & item : list)
            answer.append(table::new_column)
                  .append(item.class_ru.value_or(" "))
                  .append(table::new_line)
                  .append(item.tube_ru.value_or(" "));
        answer.append(table::new_row);
        
        answer.append("калибр");
        for (torpedo_tubes_t const & item : list)
            answer.append(table::new_column)
                  .append(item.caliber? to_string_10(*item.caliber) + "мм" : " ");
        answer.append(table::new_row);
        
        answer.append("количество труб");
        for (torpedo_tubes_t const & item : list)
            answer.append(table::new_column)
                  .append(item.tubes_count? std::to_string(*item.tubes_count) : " ");
        answer.append(table::new_row);
        
        answer.append("на вооружении");
        for (torpedo_tubes_t const & item : list)
            answer.append(table::new_column)
                  .append(item.in_service? to_string(*item.in_service) : " ");
        
        answer.append(table::end);
    }

    return answer;
}


