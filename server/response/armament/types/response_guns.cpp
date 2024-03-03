#include "response_guns.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"
#include "base_comparators.h"
#include "html_template.h"
#include "append_row.h"

static const constexpr pictures_template pictures =
{
    {
        "<li><a href=\"/pictures/gun/",
        "\"><img src=\"/pictures_small/gun/",
    }
};

struct guns_cmp
{
    static registrator_cmp <guns::guns_partial> sort;
    static registrator_cmp <guns::guns_partial> group;
    static registrator_pred <guns::guns_partial> & filter ();
};

registrator_cmp <guns::guns_partial> guns_cmp::sort
({
    { "name_ru",    comparators::name_ru <guns::guns_partial> },
    { "name_en",    comparators::name_en <guns::guns_partial> },
    { "class",      comparators::in_service <guns::guns_partial> },
    { "in_service", comparators::in_service <guns::guns_partial> },
    { "caliber",    comparators::caliber <guns::guns_partial> },
});

registrator_cmp <guns::guns_partial> guns_cmp::group
({
    { "class",      comparators::classes <guns::guns_partial> },
    { "in_service", comparators::in_service_10th <guns::guns_partial> },
    { "caliber",    comparators::universal <guns::guns_partial, int, &guns::guns_partial::caliber_group> },
});

registrator_pred <guns::guns_partial> & guns_cmp::filter ()
{
    static registrator_pred <guns::guns_partial> answer;
    if (answer.empty())
    {
        answer.reg <year_filter     <guns::guns_partial> > ("in_service");
        answer.reg <caliber_filter  <guns::guns_partial> > ("caliber");
        answer.reg <class_filter    <guns::guns_partial> > ("class");
        answer.reg <id_filter       <guns::guns_partial> > ("id");
    }
    
    return answer;
}


void guns::response (simple_string & answer, std::string_view query, piece_t title)
{
    static const constexpr std::string_view title_text = "японские орудия";
    answer.rewrite(title.position, title_text.substr(0, std::min(title_text.size(), title.size)));

    std::vector <std::vector <guns_partial> > list_group = 
         parse_group_and_sort <guns_partial, guns_cmp> (guns_cache, query);

    for (std::vector <guns_partial> const & list : list_group)
    {
        answer.append(table::begin);
        
        append_row <guns_text, &guns_text::name> (answer, list, text_cache);
        
        answer.append("калибр");
        append_row <guns_text, &guns_text::caliber> (answer, list, text_cache);
        
        answer.append("длина ствола");
        append_row <guns_text, &guns_text::length> (answer, list, text_cache);
        
        answer.append("скорострельность");
        append_row <guns_text, &guns_text::rate_of_fire> (answer, list, text_cache);
        
        answer.append("эффективная дальность");
        append_row <guns_text, &guns_text::effective_range> (answer, list, text_cache);
        
        answer.append("масса");
        append_row <guns_text, &guns_text::mass> (answer, list, text_cache);
        
        answer.append("построено");
        append_row <guns_text, &guns_text::build_cnt> (answer, list, text_cache);
        
        answer.append("на вооружении");
        for (guns_partial const & item : list)
            answer.append(text_cache[item.index].in_service);
        
        answer.append(table::end);

        add_pictures_t add_pictures(answer, pictures);
        for (guns_partial const & item : list)
            for (picture_t const & picture : pictures_cache[item.index])
                add_pictures(picture);
        add_pictures.close();
    }
}


guns::guns_partial::guns_partial (guns_t const & value, size_t _index) :
    index(_index),
    id          (value.id),
    class_id    (value.class_id),
    name_ru     (value.gun_ru),
    name_en     (value.gun_en),
    caliber     (value.caliber.value_or(std::numeric_limits <double> ::infinity())),
    caliber_group(value.caliber?
        std::floor((std::log(*value.caliber + 1.) + 0.5) / 0.3) :
        std::numeric_limits <int> ::max()),
    in_service  (value.in_service)
{}

guns::guns_text::guns_text (guns_t const & item) :
    name        (table::new_column),
    caliber     (table::new_column),
    length      (table::new_column),
    rate_of_fire(table::new_column),
    effective_range(table::new_column),
    mass        (table::new_column),
    build_cnt   (table::new_column),
    in_service  (table::new_column)
{
    name.append(item.class_ru.value_or(" "))
        .append(table::new_line)
        .append(item.gun_ru.value_or(" "));
    caliber     .append(item.caliber? to_string_10(*item.caliber) + "мм" : " ");
    length      .append(item.length? to_string_10(*item.length) + "калибров" : " ");
    rate_of_fire.append(item.rate_of_fire? to_string_10(*item.rate_of_fire) + "выстр/мин" : " ");
    effective_range.append(item.effective_range? to_string_10(*item.effective_range) + "м" : " ");
    mass        .append(item.mass? to_string_10(*item.mass) + "кг" : " ");
    build_cnt   .append(item.build_cnt? to_string_10(*item.build_cnt) + "шт" : " ");
    in_service  .append(item.in_service? to_string(*item.in_service) : " ");
}


