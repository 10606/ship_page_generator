#include "response_torpedo.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"


namespace torpedo_cmp
{

registrator_cmp <torpedo::torpedo_t> sort
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


registrator_cmp <torpedo::torpedo_t> group
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


registrator_pred <torpedo::torpedo_t> & filter ()
{
    static registrator_pred <torpedo::torpedo_t> answer;
    if (answer.empty())
    {
        answer.reg <year_filter <torpedo::torpedo_t> > ("in_service");
        answer.reg <caliber_filter <torpedo::torpedo_t> > ("caliber");
    }
    
    return answer;
}

};


std::vector <std::vector <torpedo::torpedo_t> >
torpedo::torpedo_group_and_sort
(std::string_view query)
{
 
    std::vector <ship_requests::armament_info_t::torpedo> torpedoes =
        database->armament_info.get_torpedo();
    
    std::vector <group> parsed = parse_query(query);
    
    std::unique_ptr <filter <torpedo_t> > predicate;
    std::unique_ptr <comparator <torpedo_t> > group_cmp;
    std::unique_ptr <comparator <torpedo_t> > sort_cmp;
    
    for (group const & cur_gr : parsed)
    {
        if (cur_gr.key == "group")
            group_cmp = torpedo_cmp::group.get(cur_gr.values, std::move(group_cmp));
        if (cur_gr.key == "sort")
            sort_cmp = torpedo_cmp::sort.get(cur_gr.values, std::move(sort_cmp));
        if (cur_gr.key == "filter")
            predicate = torpedo_cmp::filter().get(cur_gr.values, std::move(predicate));
    }

    return  group_and_sort
    (
        std::move(torpedoes), 
        filter_for_sort <torpedo_t> (predicate.get()),
        comparator_for_sort <torpedo_t> (group_cmp.get()), 
        comparator_for_sort <torpedo_t> (sort_cmp.get())
    );
}


std::string torpedo::response (std::string_view query)
{
    std::string answer;
    
    std::vector <std::vector <torpedo_t> > list_group = 
        torpedo_group_and_sort(query);

    for (std::vector <torpedo_t> const & list : list_group)
    {
        answer.append(table::begin);
        
        for (torpedo_t const & item : list)
            answer.append(table::new_column).append(item.torpedo_ru.value_or(" "));
        answer.append(table::new_row);
        
        answer.append("калибр");
        for (torpedo_t const & item : list)
            answer.append(table::new_column).append(item.caliber? to_string_10(*item.caliber) + "мм" : " ");
        answer.append(table::new_row);
        
        answer.append("длина");
        for (torpedo_t const & item : list)
            answer.append(table::new_column).append(item.length? to_string_10(*item.length) + "м" : " ");
        answer.append(table::new_row);
        
        answer.append("скорость");
        for (torpedo_t const & item : list)
            answer.append(table::new_column).append(item.speed? to_string_10(*item.speed) + "узлов" : " ");
        answer.append(table::new_row);
        
        answer.append("дальность");
        for (torpedo_t const & item : list)
            answer.append(table::new_column).append(item.range? to_string_10(*item.range) + "м" : " ");
        answer.append(table::new_row);
        
        answer.append("масса");
        for (torpedo_t const & item : list)
            answer.append(table::new_column).append(item.mass? to_string_10(*item.mass) + "кг" : " ");
        answer.append(table::new_row);
        
        answer.append("масса ВВ");
        for (torpedo_t const & item : list)
            answer.append(table::new_column).append(item.mass_ex? to_string_10(*item.mass_ex) + "кг" : " ");
        answer.append(table::new_row);
        
        answer.append("на вооружении");
        for (torpedo_t const & item : list)
            answer.append(table::new_column).append(item.in_service? to_string(*item.in_service) : " ");
        
        answer.append(table::end);
    }

    return answer;
}


