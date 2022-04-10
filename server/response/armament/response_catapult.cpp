#include "response_catapult.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"


struct catapult_cmp
{
    static registrator_cmp <catapult::catapult_t> sort;
    static registrator_cmp <catapult::catapult_t> group;
    static registrator_pred <catapult::catapult_t> & filter ();
};

registrator_cmp <catapult::catapult_t> catapult_cmp::sort
({
    {
        "in_service", 
        [] (catapult::catapult_t const & a, catapult::catapult_t const & b)
            { return compare_null_last(a.in_service, b.in_service); }
    },
    {
        "acceleration", 
        [] (catapult::catapult_t const & a, catapult::catapult_t const & b)
            { return compare_null_last(a.alleceration, b.alleceration); }
    },
    {
        "speed", 
        [] (catapult::catapult_t const & a, catapult::catapult_t const & b)
            { return compare_null_last(a.speed, b.speed); }
    },
    {
        "launch_mass", 
        [] (catapult::catapult_t const & a, catapult::catapult_t const & b)
            { return compare_null_last(a.launch_mass, b.launch_mass); }
    },
    {
        "name_ru", 
        [] (catapult::catapult_t const & a, catapult::catapult_t const & b)
            { return compare_null_last(a.catapult_ru, b.catapult_ru); }
    },
    {
        "name_en", 
        [] (catapult::catapult_t const & a, catapult::catapult_t const & b)
            { return compare_null_last(a.catapult_en, b.catapult_en); }
    },
    {
        "class", 
        [] (catapult::catapult_t const & a, catapult::catapult_t const & b)
            { return a.class_id <=> b.class_id; }
    },
});

registrator_cmp <catapult::catapult_t> catapult_cmp::group
({
    {
        "in_service", 
        [] (catapult::catapult_t const & a, catapult::catapult_t const & b)
            { return compare_date_10th(a.in_service, b.in_service); }
    },
    {
        "class", 
        [] (catapult::catapult_t const & a, catapult::catapult_t const & b)
            { return a.class_id <=> b.class_id; }
    },
});

registrator_pred <catapult::catapult_t> & catapult_cmp::filter ()
{
    static registrator_pred <catapult::catapult_t> answer;
    if (answer.empty())
    {
        answer.reg <year_filter <catapult::catapult_t> > ("in_service");
        answer.reg <class_filter <catapult::catapult_t> > ("class");
    }
    
    return answer;
}


std::string catapult::response (std::string_view query)
{
    std::string answer;
    
    std::vector <std::vector <catapult_t> > list_group = 
         parse_group_and_sort <catapult_t, catapult_cmp> (database->armament_info.get_catapult(), query);

    for (std::vector <catapult_t> const & list : list_group)
    {
        answer.append(table::begin);
        
        for (catapult_t const & item : list)
            answer.append(table::new_column)
                  .append(item.class_ru.value_or(" "))
                  .append(table::new_line)
                  .append(item.catapult_ru.value_or(" "));
        answer.append(table::new_row);
        
        answer.append("длина");
        for (catapult_t const & item : list)
            answer.append(table::new_column)
                  .append(item.length? to_string_10(*item.length) + "м" : " ");
        answer.append(table::new_row);
        
        answer.append("ширина");
        for (catapult_t const & item : list)
            answer.append(table::new_column)
                  .append(item.width? to_string_10(*item.width) + "м" : " ");
        answer.append(table::new_row);
        
        answer.append("скорость");
        for (catapult_t const & item : list)
            answer.append(table::new_column)
                  .append(item.speed? to_string_10(*item.speed) + "м/с" : " ");
        answer.append(table::new_row);
        
        answer.append("запускаемая масса");
        for (catapult_t const & item : list)
            answer.append(table::new_column)
                  .append(item.launch_mass? to_string_10(*item.launch_mass) + "кг" : " ");
        answer.append(table::new_row);
        
        answer.append("ускорение");
        for (catapult_t const & item : list)
            answer.append(table::new_column)
                  .append(item.alleceration? to_string_10(*item.alleceration) + "g" : " ");
        answer.append(table::new_row);
        
        answer.append("на вооружении");
        for (catapult_t const & item : list)
            answer.append(table::new_column)
                  .append(item.in_service? to_string(*item.in_service) : " ");
        
        answer.append(table::end);
    }

    return answer;
}


