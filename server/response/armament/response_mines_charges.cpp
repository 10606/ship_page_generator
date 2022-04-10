#include "response_mines_charges.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"


struct mines_charges_cmp
{
    static registrator_cmp <mines_charges::mines_charges_t> sort;
    static registrator_cmp <mines_charges::mines_charges_t> group;
    static registrator_pred <mines_charges::mines_charges_t> & filter ();
};

registrator_cmp <mines_charges::mines_charges_t> mines_charges_cmp::sort
({
    {
        "in_service", 
        [] (mines_charges::mines_charges_t const & a, mines_charges::mines_charges_t const & b)
            { return compare_null_last(a.in_service, b.in_service); }
    },
    {
        "mass_ex", 
        [] (mines_charges::mines_charges_t const & a, mines_charges::mines_charges_t const & b)
            { return compare_null_last(a.mass_ex, b.mass_ex); }
    },
    {
        "name_ru", 
        [] (mines_charges::mines_charges_t const & a, mines_charges::mines_charges_t const & b)
            { return compare_null_last(a.mine_ru, b.mine_ru); }
    },
    {
        "name_en", 
        [] (mines_charges::mines_charges_t const & a, mines_charges::mines_charges_t const & b)
            { return compare_null_last(a.mine_en, b.mine_en); }
    },
    {
        "class", 
        [] (mines_charges::mines_charges_t const & a, mines_charges::mines_charges_t const & b)
            { return a.class_id <=> b.class_id; }
    },
});

registrator_cmp <mines_charges::mines_charges_t> mines_charges_cmp::group
({
    {
        "in_service", 
        [] (mines_charges::mines_charges_t const & a, mines_charges::mines_charges_t const & b)
            { return compare_date_10th(a.in_service, b.in_service); }
    },
    {
        "class", 
        [] (mines_charges::mines_charges_t const & a, mines_charges::mines_charges_t const & b)
            { return a.class_id <=> b.class_id; }
    },
});

registrator_pred <mines_charges::mines_charges_t> & mines_charges_cmp::filter ()
{
    static registrator_pred <mines_charges::mines_charges_t> answer;
    if (answer.empty())
    {
        answer.reg <year_filter <mines_charges::mines_charges_t> > ("in_service");
        answer.reg <class_filter <mines_charges::mines_charges_t> > ("class");
    }
    
    return answer;
}


std::string mines_charges::response (std::string_view query)
{
    std::string answer;
    
    std::vector <std::vector <mines_charges_t> > list_group = 
         parse_group_and_sort <mines_charges_t, mines_charges_cmp> (database->armament_info.get_mines_charges(), query);

    for (std::vector <mines_charges_t> const & list : list_group)
    {
        answer.append(table::begin);
        
        for (mines_charges_t const & item : list)
            answer.append(table::new_column)
                  .append(item.class_ru.value_or(" "))
                  .append(table::new_line)
                  .append(item.mine_ru.value_or(" "));
        answer.append(table::new_row);
        
        answer.append("масса");
        for (mines_charges_t const & item : list)
            answer.append(table::new_column)
                  .append(item.mass? to_string_10(*item.mass) + "кг" : " ");
        answer.append(table::new_row);
        
        answer.append("масса ВВ");
        for (mines_charges_t const & item : list)
            answer.append(table::new_column)
                  .append(item.mass_ex? to_string_10(*item.mass_ex) + "кг" : " ");
        answer.append(table::new_row);
        
        answer.append("размер");
        for (mines_charges_t const & item : list)
            answer.append(table::new_column)
                  .append(item.size? to_string_10(*item.size) + "мм" : " ");
        answer.append(table::new_row);
        
        answer.append("на вооружении");
        for (mines_charges_t const & item : list)
            answer.append(table::new_column)
                  .append(item.in_service? to_string(*item.in_service) : " ");
        
        answer.append(table::end);
    }

    return answer;
}


