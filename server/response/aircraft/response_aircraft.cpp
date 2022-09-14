#include "response_aircraft.h"

#include "armament_info.h"
#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"
#include "html_template.h"
#include "base_comparators.h"
#include "append_row.h"

static const constexpr pictures_template pictures =
{
    {
        "<li><a href=\"/pictures/aircraft/",
        "\"><img src=\"/pictures/aircraft/",
        "\"></a><br>",
        "</li>"
    },
    {
        "<ul>",
        "</ul><br>"
    }
};

struct aircraft_cmp
{
    static registrator_cmp <aircraft::aircraft_partial> sort;
    static registrator_cmp <aircraft::aircraft_partial> group;
    static registrator_pred <aircraft::aircraft_partial> & filter ();
};

registrator_cmp <aircraft::aircraft_partial> aircraft_cmp::sort
({
    { "class",          comparators::classes <aircraft::aircraft_partial> },
    { "in_service",     comparators::in_service <aircraft::aircraft_partial> },
    { "type",           comparators::universal <aircraft::aircraft_partial, int, &aircraft::aircraft_partial::type_id> },
    { "mass",           comparators::universal <aircraft::aircraft_partial, double, &aircraft::aircraft_partial::mass> },
    { "engine_power",   comparators::universal <aircraft::aircraft_partial, double, &aircraft::aircraft_partial::engine_power> },
    { "max_speed",      comparators::universal <aircraft::aircraft_partial, double, &aircraft::aircraft_partial::max_speed> },
    { "range",          comparators::universal <aircraft::aircraft_partial, double, &aircraft::aircraft_partial::range> },
    { "ceiling",        comparators::universal <aircraft::aircraft_partial, double, &aircraft::aircraft_partial::ceiling> },
    { "time_alt",       comparators::universal <aircraft::aircraft_partial, double, &aircraft::aircraft_partial::time_to_altitude> },
    {
        "name_ru",
        [] (aircraft::aircraft_partial const & a, aircraft::aircraft_partial const & b)
            { return compare_null_last(a.aircraft_ru, b.aircraft_ru); }
    },
    {
        "name_en",
        [] (aircraft::aircraft_partial const & a, aircraft::aircraft_partial const & b)
            { return compare_null_last(a.aircraft_en, b.aircraft_en); }
    },
});

registrator_cmp <aircraft::aircraft_partial> aircraft_cmp::group
({
    { "class",      comparators::classes <aircraft::aircraft_partial> },
    { "in_service", comparators::in_service_10th <aircraft::aircraft_partial> },
    { "type",       comparators::universal <aircraft::aircraft_partial, int, &aircraft::aircraft_partial::type_id> },
});

registrator_pred <aircraft::aircraft_partial> & aircraft_cmp::filter ()
{
    static registrator_pred <aircraft::aircraft_partial> answer;
    if (answer.empty())
    {
        answer.reg <year_filter  <aircraft::aircraft_partial> > ("in_service");
        answer.reg <class_filter <aircraft::aircraft_partial> > ("class");
        answer.reg <type_filter  <aircraft::aircraft_partial> > ("type");
        answer.reg <id_filter    <aircraft::aircraft_partial> > ("id");
    }
    
    return answer;
}


void aircraft::response (simple_string & answer, std::string_view query, piece_t title)
{
    static const constexpr std::string_view title_text = "японская авиация";
    answer.rewrite(title.position, title_text.substr(0, std::min(title_text.size(), title.size)));

    std::vector <std::vector <aircraft_partial> > list_group = 
         parse_group_and_sort <aircraft_partial, aircraft_cmp> (aircraft_cache, query);

    for (std::vector <aircraft_partial> const & list : list_group)
    {
        answer.append(table::begin);
        
        append_row <aircraft_text, &aircraft_text::name> (answer, list, text_cache);
        
        answer.append("экипаж");
        append_row <aircraft_text, &aircraft_text::crew> (answer, list, text_cache);
        
        answer.append("масса");
        append_row <aircraft_text, &aircraft_text::mass> (answer, list, text_cache);
        
        answer.append("макс. масса");
        append_row <aircraft_text, &aircraft_text::max_mass> (answer, list, text_cache);
        
        answer.append("мощность двигателя");
        append_row <aircraft_text, &aircraft_text::engine_power> (answer, list, text_cache);
        
        answer.append("макс. скорость");
        append_row <aircraft_text, &aircraft_text::max_speed> (answer, list, text_cache);
        
        answer.append("крейсерская скорость");
        append_row <aircraft_text, &aircraft_text::cruise_speed> (answer, list, text_cache);
        
        answer.append("дальность");
        append_row <aircraft_text, &aircraft_text::range> (answer, list, text_cache);
        
        answer.append("дальность с баками");
        append_row <aircraft_text, &aircraft_text::range_with_tank> (answer, list, text_cache);
        
        answer.append("потолок");
        append_row <aircraft_text, &aircraft_text::ceiling> (answer, list, text_cache);
        
        answer.append("сокроподьемность");
        append_row <aircraft_text, &aircraft_text::time_to_altitude> (answer, list, text_cache);
        
        answer.append("на вооружении");
        append_row <aircraft_text, &aircraft_text::in_service> (answer, list, text_cache);
        
        answer.append("вооружение");
        for (aircraft_partial const & item : list)
        {
            answer.append(table::new_column);
            for (size_t i = 0; i != armament[item.index].size(); ++i)
            {
                add_value(answer, armament[item.index][i].count);
                answer.append(guns[armament[item.index][i].gun_index]);
            }
        }

        answer.append(table::end);

        add_pictures_t add_pictures(answer, pictures);
        for (aircraft_partial const & item : list)
            for (picture_t const & picture : pictures_cache[item.index])
                add_pictures(picture);
        add_pictures.close();
    }
}


aircraft::aircraft_text::aircraft_text (aircraft_t const & value) :
    name        (table::new_column),
    crew        (table::new_column),
    mass        (table::new_column),
    max_mass    (table::new_column),
    engine_power(table::new_column),
    max_speed   (table::new_column),
    cruise_speed(table::new_column),
    range       (table::new_column),
    range_with_tank (table::new_column),
    ceiling         (table::new_column),
    time_to_altitude(table::new_column),
    in_service      (table::new_column)
{
    name.append(value.class_ru.value_or(" "))
        .append(table::new_line)
        .append(value.aircraft_en.value_or(" "));
    crew.append(value.crew? std::to_string(*value.crew) + "чел" : " ");
    mass.append(value.mass?   to_string_10(*value.mass) + "кг"  : " ");
    max_mass    .append(value.max_mass?     to_string_10(*value.max_mass)       + "кг"   : " ");
    engine_power.append(value.engine_power? to_string_10(*value.engine_power)   + "л.с." : " ");
    max_speed   .append(value.max_speed?    to_string_10(*value.max_speed)      + "км/ч" : " ");
    cruise_speed.append(value.cruise_speed? to_string_10(*value.cruise_speed)   + "км/ч" : " ");
    range       .append(value.range?        to_string_10(*value.range)          + "км"   : " ");
    range_with_tank .append(value.range_with_tank?  to_string_10(*value.range_with_tank)    + "км" : " ");
    ceiling         .append(value.ceiling?          to_string_10(*value.ceiling)            + "км" : " ");
    time_to_altitude.append(value.time_to_altitude? to_string_10(*value.time_to_altitude)   + "мин/км" : " ");
    in_service      .append(value.in_service?       to_string   (*value.in_service) : " ");
}


aircraft::aircraft_partial::aircraft_partial (aircraft_t const & value, size_t _index) :
    index(_index),
    id          (value.id),
    type_id     (value.type_id),
    class_id    (value.class_id),
    aircraft_ru (value.aircraft_ru),
    aircraft_en (value.aircraft_en),
    mass        (value.mass         .value_or(std::numeric_limits <double> ::infinity())),
    engine_power(value.engine_power .value_or(std::numeric_limits <double> ::infinity())),
    max_speed   (value.max_speed    .value_or(std::numeric_limits <double> ::infinity())),
    range       (value.range        .value_or(std::numeric_limits <double> ::infinity())),
    range_with_tank (value.range_with_tank  .value_or(std::numeric_limits <double> ::infinity())),
    ceiling         (value.ceiling          .value_or(std::numeric_limits <double> ::infinity())),
    time_to_altitude(value.time_to_altitude .value_or(std::numeric_limits <double> ::infinity())),
    in_service      (value.in_service)
{}


void aircraft::fill_aircraft_armament
(
    ship_requests * database,
    std::vector <aircraft_t> const & aircraft_list
)
{
    typedef ship_requests::armament_info_t::list gun_t;
    typedef ship_requests::aircraft_info_t::guns guns_raw_t;
    std::vector <gun_t> guns_list =
        database->armament_info.get_list();

    std::vector <guns_raw_t> guns_raw =
        database->aircraft_info.get_guns();
    
    std::set <int> used;
    for (guns_raw_t const & gun : guns_raw)
        used.insert(gun.gun_id);

    std::unordered_map <int, size_t> gun_index;
    guns.reserve(used.size());
    for (gun_t & gun : guns_list)
    {
        int gun_id = gun.id;
        if (used.find(gun_id) == used.end())
            continue;
        gun_index.insert({gun_id, guns.size()});

        std::string answer = " ";
        if (gun.caliber)
            answer.append(to_string_10(*gun.caliber) + "мм");
        answer.append("  ");
        answer += gun.gun_ru.value_or("  ");
        answer.append(table::new_line);
        guns.push_back(answer);
    }

    std::unordered_map <int, size_t> aircraft_index;
    for (size_t i = 0; i != aircraft_list.size(); ++i)
        aircraft_index.insert({aircraft_list[i].id, i});

    armament.resize(aircraft_list.size());
    for (guns_raw_t & gun : guns_raw)
    {
        std::unordered_map <int, size_t> ::iterator it_gun = gun_index.find(gun.gun_id);
        std::unordered_map <int, size_t> ::iterator it_aircraft = aircraft_index.find(gun.aircraft_id);
        if (it_gun != gun_index.end() &&
            it_aircraft != aircraft_index.end())
            armament[it_aircraft->second].emplace_back(it_gun->second, gun.count);
    }
}

