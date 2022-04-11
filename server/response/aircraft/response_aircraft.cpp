#include "response_aircraft.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"


struct aircraft_cmp
{
    static registrator_cmp <aircraft::aircraft_t> sort;
    static registrator_cmp <aircraft::aircraft_t> group;
    static registrator_pred <aircraft::aircraft_t> & filter ();
};

registrator_cmp <aircraft::aircraft_t> aircraft_cmp::sort
({
    {
        "in_service", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return compare_null_last(a.in_service, b.in_service); }
    },
    {
        "mass", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return compare_null_last(a.mass, b.mass); }
    },
    {
        "engine_power", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return compare_null_last(a.engine_power, b.engine_power); }
    },
    {
        "max_speed", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return compare_null_last(a.max_speed, b.max_speed); }
    },
    {
        "range", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return compare_null_last(a.range, b.range); }
    },
    {
        "ceiling", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return compare_null_last(a.ceiling, b.ceiling); }
    },
    {
        "time_alt", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return compare_null_last(a.time_to_altitude, b.time_to_altitude); }
    },
    {
        "name_ru", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return compare_null_last(a.aircraft_ru, b.aircraft_ru); }
    },
    {
        "name_en", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return compare_null_last(a.aircraft_en, b.aircraft_en); }
    },
    {
        "class", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return a.class_id <=> b.class_id; }
    },
    {
        "type", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return a.type_id <=> b.type_id; }
    },
});

registrator_cmp <aircraft::aircraft_t> aircraft_cmp::group
({
    {
        "in_service", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return compare_date_10th(a.in_service, b.in_service); }
    },
    {
        "class", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return a.class_id <=> b.class_id; }
    },
    {
        "type", 
        [] (aircraft::aircraft_t const & a, aircraft::aircraft_t const & b)
            { return a.type_id <=> b.type_id; }
    },
});

registrator_pred <aircraft::aircraft_t> & aircraft_cmp::filter ()
{
    static registrator_pred <aircraft::aircraft_t> answer;
    if (answer.empty())
    {
        answer.reg <year_filter  <aircraft::aircraft_t> > ("in_service");
        answer.reg <class_filter <aircraft::aircraft_t> > ("class");
        answer.reg <type_filter <aircraft::aircraft_t> > ("type");
        answer.reg <id_filter    <aircraft::aircraft_t> > ("id");
    }
    
    return answer;
}


std::string aircraft::response (std::string_view query)
{
    std::string answer;
    
    std::vector <std::vector <aircraft_t> > list_group = 
         parse_group_and_sort <aircraft_t, aircraft_cmp> (database->aircraft_info.get_list(""), query);

    for (std::vector <aircraft_t> const & list : list_group)
    {
        answer.append(table::begin);
        
        for (aircraft_t const & item : list)
            answer.append(table::new_column)
                  .append(item.class_ru.value_or(" "))
                  .append(table::new_line)
                  .append(item.aircraft_en.value_or(" "));
        answer.append(table::new_row);
        
        answer.append("экипаж");
        for (aircraft_t const & item : list)
            answer.append(table::new_column)
                  .append(item.crew? std::to_string(*item.crew) + "чел" : " ");
        answer.append(table::new_row);
        
        answer.append("масса");
        for (aircraft_t const & item : list)
            answer.append(table::new_column)
                  .append(item.mass? to_string_10(*item.mass) + "кг" : " ");
        answer.append(table::new_row);
        
        answer.append("макс. масса");
        for (aircraft_t const & item : list)
            answer.append(table::new_column)
                  .append(item.max_mass? to_string_10(*item.max_mass) + "кг" : " ");
        answer.append(table::new_row);
        
        answer.append("мощность двигателя");
        for (aircraft_t const & item : list)
            answer.append(table::new_column)
                  .append(item.engine_power? to_string_10(*item.engine_power) + "л.с." : " ");
        answer.append(table::new_row);
        
        answer.append("макс. скорость");
        for (aircraft_t const & item : list)
            answer.append(table::new_column)
                  .append(item.max_speed? to_string_10(*item.max_speed) + "км/ч" : " ");
        answer.append(table::new_row);
        
        answer.append("крейсерская скорость");
        for (aircraft_t const & item : list)
            answer.append(table::new_column)
                  .append(item.cruise_speed? to_string_10(*item.cruise_speed) + "км/ч" : " ");
        answer.append(table::new_row);
        
        answer.append("дальность");
        for (aircraft_t const & item : list)
            answer.append(table::new_column)
                  .append(item.range? to_string_10(*item.range) + "км" : " ");
        answer.append(table::new_row);
        
        answer.append("дальность с баками");
        for (aircraft_t const & item : list)
            answer.append(table::new_column)
                  .append(item.range_with_tank? to_string_10(*item.range_with_tank) + "км" : " ");
        answer.append(table::new_row);
        
        answer.append("потолок");
        for (aircraft_t const & item : list)
            answer.append(table::new_column)
                  .append(item.ceiling? to_string_10(*item.ceiling) + "км" : " ");
        answer.append(table::new_row);
        
        answer.append("сокроподьемность");
        for (aircraft_t const & item : list)
            answer.append(table::new_column)
                  .append(item.time_to_altitude? to_string_10(*item.time_to_altitude) + "мин/км" : " ");
        answer.append(table::new_row);
        
        answer.append("на вооружении");
        for (aircraft_t const & item : list)
            answer.append(table::new_column)
                  .append(item.in_service? to_string(*item.in_service) : " ");
        
        answer.append(table::end);
    }

    return answer;
}


