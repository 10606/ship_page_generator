#include "response_mines_charges.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"
#include "base_comparators.h"
#include "append_row.h"


struct mines_charges_cmp
{
    static registrator_cmp <mines_charges::mines_charges_partial> sort;
    static registrator_cmp <mines_charges::mines_charges_partial> group;
    static registrator_pred <mines_charges::mines_charges_partial> & filter ();
};

registrator_cmp <mines_charges::mines_charges_partial> mines_charges_cmp::sort
({
    { "name_ru",    comparators::name_ru <mines_charges::mines_charges_partial> },
    { "name_en",    comparators::name_en <mines_charges::mines_charges_partial> },
    { "class",      comparators::classes <mines_charges::mines_charges_partial> },
    { "in_service", comparators::in_service <mines_charges::mines_charges_partial> },
    { "mass_ex",    comparators::universal <mines_charges::mines_charges_partial, double, &mines_charges::mines_charges_partial::mass_ex> },
});

registrator_cmp <mines_charges::mines_charges_partial> mines_charges_cmp::group
({
    { "class",      comparators::classes <mines_charges::mines_charges_partial> },
    { "in_service", comparators::in_service_10th <mines_charges::mines_charges_partial> },
});

registrator_pred <mines_charges::mines_charges_partial> & mines_charges_cmp::filter ()
{
    static registrator_pred <mines_charges::mines_charges_partial> answer;
    if (answer.empty())
    {
        answer.reg <year_filter  <mines_charges::mines_charges_partial> > ("in_service");
        answer.reg <class_filter <mines_charges::mines_charges_partial> > ("class");
        answer.reg <id_filter    <mines_charges::mines_charges_partial> > ("id");
    }
    
    return answer;
}


void mines_charges::response (simple_string & answer, std::string_view query, piece_t title)
{
    static const constexpr std::string_view title_text = "японские мины и глубинные бомбы";
    answer.rewrite(title.position, title_text.substr(0, std::min(title_text.size(), title.size)));

    std::vector <std::vector <mines_charges_partial> > list_group = 
         parse_group_and_sort <mines_charges_partial, mines_charges_cmp> (mines_charges_cache, query);

    for (std::vector <mines_charges_partial> const & list : list_group)
    {
        answer.append(table::begin);
        
        append_row <mines_charges_text, &mines_charges_text::name> (answer, list, text_cache);
        
        answer.append("масса");
        append_row <mines_charges_text, &mines_charges_text::mass> (answer, list, text_cache);
        
        answer.append("масса ВВ");
        append_row <mines_charges_text, &mines_charges_text::mass_ex> (answer, list, text_cache);
        
        answer.append("размер");
        append_row <mines_charges_text, &mines_charges_text::size> (answer, list, text_cache);
        
        answer.append("на вооружении");
        append_row <mines_charges_text, &mines_charges_text::in_service, 0> (answer, list, text_cache);
        
        answer.append(table::end);
        answer.append("<br>");
    }
}


mines_charges::mines_charges_partial::mines_charges_partial (mines_charges_t const & value, size_t _index) :
    index(_index),
    id          (value.id),
    class_id    (value.class_id),
    name_ru     (), // filled later as pointer to mines_charges_cache
    name_en     (), // filled later as pointer to mines_charges_cache
    mass_ex     (value.mass_ex.value_or(std::numeric_limits <double> ::infinity())),
    in_service  (value.in_service)
{}

mines_charges::mines_charges_text::mines_charges_text (mines_charges_t const & item) :
    name    (table::new_column),
    name_ru (item.mine_ru),
    name_en (item.mine_en),
    mass    (table::new_column),
    mass_ex (table::new_column),
    size    (table::new_column),
    in_service(table::new_column)
{
    name.append(item.class_ru.value_or(" "))
        .append(table::new_line)
        .append(item.mine_ru.value_or(" "));
    mass    .append(item.mass? to_string_10(*item.mass) + "кг" : " ");
    mass_ex .append(item.mass_ex? to_string_10(*item.mass_ex) + "кг" : " ");
    size    .append(item.size? to_string_10(*item.size) + "мм" : " ");
    in_service.append(item.in_service? to_string(*item.in_service) : " ");
}


