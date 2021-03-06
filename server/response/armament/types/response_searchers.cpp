#include "response_searchers.h"

#include "group_and_sorting.h"
#include "parse_query.h"
#include "table.h"
#include "date_to_str.h"
#include "registrators.h"
#include "base_compare_predict.h"
#include "base_comparators.h"
#include "html_template.h"

static const constexpr pictures_template pictures =
{
    {
        "<li><a href=\"/pictures/searcher/",
        "\"><img src=\"/pictures/searcher/",
        "\"></a><br>",
        "</li>"
    },
    {
        "<ul>",
        "</ul><br>"
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
    {
        "in_service", comparators::in_service <searcher::searchers_partial>
    },
    {
        "mass", 
        [] (searcher::searchers_partial const & a, searcher::searchers_partial const & b)
            { return a.mass <=> b.mass; }
    },
    {
        "power", 
        [] (searcher::searchers_partial const & a, searcher::searchers_partial const & b)
            { return a.power <=> b.power; }
    },
    {
        "name_ru", comparators::name_ru <searcher::searchers_partial>
    },
    {
        "name_en", comparators::name_en <searcher::searchers_partial>
    },
    {
        "class", comparators::classes <searcher::searchers_partial>
    },
});

registrator_cmp <searcher::searchers_partial> searcher_cmp::group
({
    {
        "in_service", comparators::in_service_10th <searcher::searchers_partial>
    },
    {
        "class", comparators::classes <searcher::searchers_partial>
    },
    {
        "power",
        [] (searcher::searchers_partial const & a, searcher::searchers_partial const & b) -> std::partial_ordering
        {
            return a.power_group <=> b.power_group;
        }
    },
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


void searcher::response (simple_string & answer, std::string_view query)
{
    std::vector <std::vector <searchers_partial> > list_group = 
         parse_group_and_sort <searchers_partial, searcher_cmp> (searchers_cache, query);

    for (std::vector <searchers_partial> const & list : list_group)
    {
        answer.append(table::begin);
        
        for (searchers_partial const & item : list)
            answer.append(text_cache[item.index].name);
        answer.append(table::new_row);
        
        answer.append("??????????");
        for (searchers_partial const & item : list)
            answer.append(text_cache[item.index].mass);
        answer.append(table::new_row);
        
        answer.append("??????????????");
        for (searchers_partial const & item : list)
            answer.append(text_cache[item.index].frequency);
        answer.append(table::new_row);
        
        answer.append("????????????????");
        for (searchers_partial const & item : list)
            answer.append(text_cache[item.index].power);
        answer.append(table::new_row);
        
        answer.append("??????????????????");
        for (searchers_partial const & item : list)
            answer.append(text_cache[item.index].build_cnt);
        answer.append(table::new_row);
        
        answer.append("???? ????????????????????");
        for (searchers_partial const & item : list)
            answer.append(text_cache[item.index].in_service);
        
        answer.append(table::end);

        add_pictures_t add_pictures(answer, pictures);
        for (searchers_partial const & item : list)
            for (picture_t const & picture : pictures_cache[item.index])
                add_pictures(picture);
        add_pictures.close();
    }
}


searcher::searchers_partial::searchers_partial (searcher_t const & value, size_t _index) :
    index(_index),
    id          (value.id),
    class_id    (value.class_id),
    name_ru     (value.searcher_ru),
    name_en     (value.searcher_en),
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
        return to_string_10(frequency) + "??????";
    frequency /= 1000;
    if (frequency < 10000) [[unlikely]]
        return to_string_10(frequency) + "??????";
    double exp = std::floor(std::log10(frequency));
    frequency /= std::exp(exp * std::log(10));
    frequency = std::round(frequency * 10) / 10;
    return to_string_10(frequency) + "<sup>" + to_string_10(exp + 9) + "</sup>????";
}

searcher::searchers_text::searchers_text (searcher_t const & item) :
    name        (table::new_column),
    mass        (table::new_column),
    frequency   (table::new_column),
    power       (table::new_column),
    build_cnt   (table::new_column),
    in_service  (table::new_column)
{
    name      .append(item.searcher_ru.value_or(" "));
    mass      .append(item.mass? to_string_10(*item.mass) + "????" : " ");
    frequency .append(item.frequency? freq_convert(*item.frequency) : " ");
    power     .append(item.power? to_string_10(*item.power) + "??????" : " ");
    build_cnt .append(item.build_cnt? to_string_10(*item.build_cnt) + "????" : " ");
    in_service.append(item.in_service? to_string(*item.in_service) : " ");
}


