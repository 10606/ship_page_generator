#include "response_searchers.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"
#include "base_comparators.h"
#include "html_template.h"
#include "append_row.h"
#include "html_view_pictures.h"


static const constexpr pictures_template pictures =
{
    {
        "<li><a href=\"/pictures/searcher/",
        "\"><img src=\"/pictures_small/searcher/"
    }
};

struct searcher_cmp
{
    static registrator_cmp <searcher::searchers_partial> sort;
    static registrator_cmp <searcher::searchers_partial> group;
    static registrator_pred <searcher::searchers_partial> & filter ();
};

registrator_cmp <searcher::searchers_partial> searcher_cmp::sort
({
    { "name_ru",    comparators::name_ru <searcher::searchers_partial> },
    { "name_en",    comparators::name_en <searcher::searchers_partial> },
    { "class",      comparators::classes <searcher::searchers_partial> },
    { "in_service", comparators::in_service <searcher::searchers_partial> },
    { "mass",       comparators::universal <searcher::searchers_partial, double, &searcher::searchers_partial::mass> },
    { "power",      comparators::universal <searcher::searchers_partial, double, &searcher::searchers_partial::power> },
});

registrator_cmp <searcher::searchers_partial> searcher_cmp::group
({
    { "class",      comparators::classes <searcher::searchers_partial> },
    { "in_service", comparators::in_service_10th <searcher::searchers_partial> },
    { "power",      comparators::universal <searcher::searchers_partial, int, &searcher::searchers_partial::power_group> },
});

registrator_pred <searcher::searchers_partial> & searcher_cmp::filter ()
{
    static registrator_pred <searcher::searchers_partial> answer;
    if (answer.empty())
    {
        answer.reg <year_filter  <searcher::searchers_partial> > ("in_service");
        answer.reg <class_filter <searcher::searchers_partial> > ("class");
        answer.reg <id_filter    <searcher::searchers_partial> > ("id");
    }
    
    return answer;
}


void searcher::response (simple_string & answer, std::string_view query, piece_t title)
{
    static const constexpr std::string_view title_text = "японские средства обнаружения";
    answer.rewrite(title.position, title_text.substr(0, std::min(title_text.size(), title.size)));

    std::vector <std::vector <searchers_partial> > list_group = 
         parse_group_and_sort <searchers_partial, searcher_cmp> (searchers_cache, query);

    answer.append(html_view_pictures);

    for (std::vector <searchers_partial> const & list : list_group)
    {
        answer.append(table::begin);
        
        append_row <searchers_text, &searchers_text::name> (answer, list, text_cache);
        
        answer.append("масса");
        append_row <searchers_text, &searchers_text::mass> (answer, list, text_cache);
        
        answer.append("частота");
        append_row <searchers_text, &searchers_text::frequency> (answer, list, text_cache);
        
        answer.append("мощность");
        append_row <searchers_text, &searchers_text::power> (answer, list, text_cache);
        
        answer.append("построено");
        append_row <searchers_text, &searchers_text::build_cnt> (answer, list, text_cache);
        
        answer.append("на вооружении");
        append_row <searchers_text, &searchers_text::in_service, 0> (answer, list, text_cache);
        
        answer.append(table::end);

        add_pictures <searchers_partial, searchers_text>
        (
            answer,
            pictures,
            list, 
            pictures_cache,
            text_cache
        );
    }
}


searcher::searchers_partial::searchers_partial (searcher_t const & value, size_t _index) :
    index(_index),
    id          (value.id),
    class_id    (value.class_id),
    name_ru     (), // filled later as pointer to searchers_cache
    name_en     (), // filled later as pointer to searchers_cache
    mass        (value.mass .value_or(std::numeric_limits <double> ::infinity())),
    power       (value.power.value_or(std::numeric_limits <double> ::infinity())),
    power_group (value.power? 
                    std::floor(std::log(*value.power + 1)) : 
                    std::numeric_limits <int> ::max()),
    in_service  (value.in_service)
{}

std::string searcher::searchers_text::freq_convert (double frequency)
{
    if (frequency < 10000) [[likely]]
        return to_string_10(frequency) + "МГц";
    frequency /= 1000;
    if (frequency < 10000) [[unlikely]]
        return to_string_10(frequency) + "ГГц";
    double exp = std::floor(std::log10(frequency));
    frequency /= std::exp(exp * std::log(10));
    frequency = std::round(frequency * 10) / 10;
    return to_string_10(frequency) + "<sup>" + to_string_10(exp + 9) + "</sup>Гц";
}

searcher::searchers_text::searchers_text (searcher_t const & item) :
    name        (table::new_column),
    name_ru     (item.searcher_ru),
    name_en     (item.searcher_en),
    mass        (table::new_column),
    frequency   (table::new_column),
    power       (table::new_column),
    build_cnt   (table::new_column),
    in_service  (table::new_column)
{
    name      .append(item.searcher_ru.value_or(" "));
    mass      .append(item.mass? to_string_10(*item.mass) + "кг" : " ");
    frequency .append(item.frequency? freq_convert(*item.frequency) : " ");
    power     .append(item.power? to_string_10(*item.power) + "кВт" : " ");
    build_cnt .append(item.build_cnt? to_string_10(*item.build_cnt) + "шт" : " ");
    in_service.append(item.in_service? to_string(*item.in_service) : " ");
}


