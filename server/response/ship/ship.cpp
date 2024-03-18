#include "ship.h"

#include "nested_segments.h"
#include "ship_requests.h"
#include "ship_event.h"
#include "ship_info.h"
#include "pictures.h"
#include "date_to_str.h"
#include "parse_query.h"


void add_modernizations
(
    std::string & answer, 
    std::vector <ship_requests::ship_event_t::event_lt_descr> const & events,
    std::vector <size_t> const & index_mapping,
    std::vector <segment> segments,
    std::optional <std::chrono::year_month_day> commisioned
)
{
    std::vector <segment> :: iterator part = 
        std::partition(segments.begin(), segments.end(), 
                        [&events, &index_mapping] (segment const & value) -> bool
                        {
                            // modernization only
                            return events[index_mapping[value.index]].class_id == 0;
                        });

    std::sort(segments.begin(), part, 
                [] (segment const & a, segment const & b) -> bool 
                { 
                    return std::is_lt(compare_null_last(a.begin, b.begin)); 
                });
    
    std::vector <std::chrono::year_month_day> modernizations;
    modernizations.reserve(part - segments.begin());

    for (std::vector <segment> :: iterator it = segments.begin(); it != part; ++it)
        if (it->end && commisioned && (*commisioned <= *it->end))
        {
            while (!modernizations.empty() && it->begin && modernizations.back() >= *it->begin)
                modernizations.pop_back();
            modernizations.push_back(*it->end);
        }

    for (auto const & date : modernizations)
        answer.append("&date=")
              .append(to_string(date));
}


struct ship::add_event
{
    add_event 
    (
        std::string & _answer,
        std::vector <ship_requests::ship_event_t::event_lt_descr> const & _events,
        std::vector <std::vector <size_t> > const & _graph,
        std::vector <size_t> const & _index_mapping
    ) :
        answer(_answer),
        events(_events),
        graph(_graph),
        index_mapping(_index_mapping),
        visited(graph.size(), 0)
    {}

    void visit (size_t vertex, size_t shift = 1)
    {
        visited[vertex] = 1;
        size_t index = index_mapping[vertex];
        
        answer.append(ship::row.begin);
        for (size_t i = 0; i != shift; ++i)
            answer.append(ship::shift);
        answer.append("<b>");
        if (events[index].date_from)
            answer.append(to_string(*events[index].date_from));
        else
            answer.append(date_placeholder);
        answer.append(" - ");
        if (events[index].date_to)
            answer.append(to_string(*events[index].date_to));
        else
            answer.append(date_placeholder);
        answer.append("</b>");
        answer.append(" ");
        if (events[index].class_ru)
            answer.append(*events[index].class_ru)
                  .append(": ");
        if (events[index].description)
            answer.append(*events[index].description);
        answer.append(ship::row.end);
        
        for (size_t next : graph[vertex])
            visit(next, shift + 1);
    }
    
    void operator () ()
    {
        std::vector <size_t> order(graph.size());
        for (size_t i = 0; i != graph.size(); ++i)
            order[i] = i;
        
        auto is_one_day_event = 
            [this] (size_t index) -> bool
            {
                ship_requests::ship_event_t::event_lt_descr const & value = events[index_mapping[index]];
                if (!value.date_from || !value.date_to)
                    return 0;
                return value.date_from == value.date_to;
            };
        
        auto comparator = 
            [this, is_one_day_event] (size_t a, size_t b) -> bool 
            { 
                std::strong_ordering begin = compare_null_last(events[index_mapping[a]].date_from, 
                                                               events[index_mapping[b]].date_from);
                if (!std::is_eq(begin))
                    return std::is_lt(begin);
                
                if (is_one_day_event(a))
                    return 1;
                if (is_one_day_event(b))
                    return 0;
                return std::is_lt(compare_null_last(events[index_mapping[b]].date_to, 
                                                    events[index_mapping[a]].date_to));
            };
        
        std::sort(order.begin(), order.end(), comparator);
        
        for (auto & list : graph)
            std::sort(list.begin(), list.end(), comparator);
        
        for (size_t vertex : order)
            if (!visited[vertex])
                visit(vertex);
    }
    
private:
    std::string & answer;
    std::vector <ship_requests::ship_event_t::event_lt_descr> const & events;
    std::vector <std::vector <size_t> > graph;
    std::vector <size_t> const & index_mapping;
    std::vector <uint8_t> visited;
    
    static const constexpr std::string_view date_placeholder = "&ensp;&ensp;&nbsp;&ensp;&ensp;&nbsp;&ensp;&ensp;&ensp;&ensp;";
};


void ship::add_general_info
(
    std::string & answer, 
    std::string & modernization_link, 
    ship_requests::ship_info_t::list const & info
)
{
    answer.append(ship::row.begin);
    if (info.ship_ru)
        answer.append("<h2 id=\"id_")
              .append(std::to_string(info.ship_id))
              .append("\">")
              .append(*info.ship_ru)
              .append("</h2> ");
    if (info.class_ru || info.type_ru)
    {
        answer.append("(");
        if (info.class_ru)
            answer.append(*info.class_ru);
        if (info.class_ru && info.type_ru)
            answer.append(" ");
        if (info.type_ru)
            answer.append("типа ")
                  .append(*info.type_ru);
        answer.append(")");
    }
    answer.append(ship::row.end);
    
    answer.append(ship::row.begin);
    answer.append("<b>");
    if (info.commissioned)
    {
        std::string commissioned_str = to_string(*info.commissioned);
        answer.append(commissioned_str);
        modernization_link.append("&date=")
                          .append(std::move(commissioned_str));
    }
    answer.append(" -> ");
    if (info.sunk_date)
        answer.append(to_string(*info.sunk_date));
    answer.append("</b>");
    if (info.sunk_reason)
    {
        answer.append(" (")
              .append(*info.sunk_reason)
              .append(")");
    }
    answer.append(ship::row.end);
}


void ship::add_short_info
(
    response_t & answer, 
    ship_requests::ship_info_t::list const & info
)
{
    answer.short_info.append("<tr><th><a href=\"#id_")
                     .append(std::to_string(info.ship_id))
                     .append("\">");
    answer.name.position = answer.short_info.size();
    answer.name.size = info.ship_ru? info.ship_ru->size() : 0;
    answer.short_info.append(info.ship_ru.value_or(""))
                     .append("</a></th><th>")
                     .append(info.commissioned? to_string(*info.commissioned) : "")
                     .append("</th><th>")
                     .append(info.sunk_date? to_string(*info.sunk_date) : "")
                     .append("</th><td>")
                     .append(info.sunk_reason? *info.sunk_reason : "")
                     .append("</td></tr>\n");
}



ship::ship (ship_requests * database, ship_armament & _armament) :
    armament(_armament),
    modernizations(),
    type_list(),
    ship_list_in_type()
{
    std::vector <ship_requests::ship_event_t::event_lt_descr> events = 
        database->ship_event.get_event_lt_descr();
    
    typedef ship_requests::ship_info_t::list list_t;
    std::vector <list_t> list =
        database->ship_info.get_list("order by ship_list.commissioned");

    typedef ship_requests::pictures_t::picture picture_t;
    std::vector <picture_t> ship_pictures_list =
        database->pictures.get_ship();

    typedef ship_requests::ship_info_t::types types_t;
    std::vector <types_t> ship_types_list =
        database->ship_info.get_types();

    
    std::unordered_map <int, std::vector <segment> > ship_to_segment;
    std::unordered_map <int, std::vector <size_t> > index_mapping; // 0..size -> event index
    std::unordered_map <int, list_t> ship_info;
    std::unordered_map <int, std::vector <picture_t> > ship_pictures;
    std::unordered_map <int, size_t> ship_types;

    type_list.push_back("");
    for (size_t i = 0; i != ship_types_list.size(); ++i)
    {
        ship_types.insert({ship_types_list[i].type_id, type_list.size()});
        type_list.push_back(ship_types_list[i].type_ru.value_or("---"));
    }
    
    for (size_t i = 0; i != events.size(); ++i)
    {
        size_t index = ship_to_segment[events[i].ship_id].size();
        ship_to_segment[events[i].ship_id].emplace_back(events[i].date_from, events[i].date_to, index);
        index_mapping[events[i].ship_id].push_back(i);
    }

    for (size_t i = 0; i != list.size(); ++i)
    {
        int ship_id = list[i].ship_id;
        ship_list_in_type[list[i].type_id].push_back(ship_id);
        ship_info.insert({ship_id, std::move(list[i])});
    }

    for (auto && item : ship_pictures_list)
        ship_pictures[item.id].push_back(std::move(item));

    for (auto & info : ship_info)
    {
        int ship_id = info.first;
        
        response_t answer;
        answer.begin = std::string("<div class=\"events\">");
        answer.armament_link = std::string(query_template);
        answer.armament_link.append(std::to_string(ship_id));

        std::unordered_map <int, size_t> ::iterator it = ship_types.find(info.second.type_id);
        answer.type = (it == ship_types.end())? 0 : it->second;

        // short info
        {
            add_short_info(answer, info.second);
        }
        
        // general info
        answer.begin.append("<table class=\"short_info\" border=0 rules=\"rows\"><tbody>\n");
        {
            add_general_info(answer.begin, answer.armament_link, info.second);
        }

        auto & ship_data = ship_to_segment[ship_id];
        std::vector <size_t> const & index_mapping_value = index_mapping[ship_id];
        
        // modernizations link
        {
            std::optional <std::chrono::year_month_day> commisioned = info.second.commissioned;
            add_modernizations(answer.armament_link, events, index_mapping_value, ship_data, commisioned);
        }
        
        // add events
        {
            std::vector <std::vector <size_t> > nested = nested_segments(std::move(ship_data));
            add_event event_adder(answer.begin, events, std::move(nested), index_mapping_value);
            event_adder();
        }
        answer.begin.append("</tbody></table>\n");
        
        answer.begin.append(link.begin)
                    .append(answer.armament_link)
                    .append(link.end)
                    .append("</div><div class=\"long_info\">\n");
        answer.end.append(new_line);
        {
            add_pictures_t add_pictures(answer.end, pictures);
            for (auto const & info : ship_pictures[ship_id])
                add_pictures(info);
            add_pictures.close();
        }
        answer.end.append("</div>")
                  .append(new_line)
                  .append(new_line);
        modernizations.insert({ship_id, std::move(answer)});
    }
}


void ship::response (simple_string & answer, std::string_view query, piece_t title)
{
    std::vector <id_or_group_t> ids = parse_query__id(query);

    std::vector <size_t> type_count(type_list.size(), 0);
    bool not_empty_title = 0;

    answer.append("<table class=\"short_info\" border=0 rules=\"rows\"><tbody>\n");
    auto short_ship_info = [this, &answer, &type_count] (int id) -> void
    {
        std::unordered_map <int, response_t> :: iterator it = modernizations.find(id);
        if (it == modernizations.end()) [[unlikely]]
            return;
        type_count[it->second.type]++;
        answer.append(it->second.short_info);
    };
    for (id_or_group_t id_or_group : ids)
    {
        if (id_or_group.type == id_or_group_t::id)
            short_ship_info(id_or_group.value);
        else
        {
            std::unordered_map <int, std::vector <int> > ::iterator it = ship_list_in_type.find(id_or_group.value);
            if (it == ship_list_in_type.end()) [[unlikely]]
                continue;
            for (int id : it->second)
                short_ship_info(id);
        }
    }
    answer.append
    (
        "</tbody></table><br>\n",
        detail_compare_ships,
        "<br><br>"
    );
    
    auto long_ship_info = [this, &answer, &type_count, &not_empty_title, &title] (int id) -> void
    {
        std::unordered_map <int, response_t> :: iterator it = modernizations.find(id);
        if (it == modernizations.end()) [[unlikely]]
            return;

        answer.append(it->second.begin);
        armament.response(answer, it->second.armament_link, {0, 0}, 1);
        answer.append(it->second.end);

        
        static const constexpr std::string_view delimeter = ", ";
        static const constexpr std::string_view and_other = "..."; 
        size_t need_size = not_empty_title? delimeter.size() : 0;

        auto add_to_title = [&answer, &title] (std::string_view value) -> void
        {
            answer.rewrite(title.position, value);
            title.position += value.size();
            title.size -= value.size();
        };

        auto add_delimeter = [&add_to_title, &not_empty_title] () -> void
        {
            if (not_empty_title)
                add_to_title(delimeter);
        };

        auto end_of_title = [&answer, &title] () -> void
        {
            if (title.size >= and_other.size())
                answer.rewrite(title.position, and_other);
            title.size = 0;
        };
        
        if (type_count[it->second.type] > 1) [[likely]]
        {
            static const constexpr std::string_view prefix = "тип ";
            if (title.size >= need_size + prefix.size() + type_list[it->second.type].size()) [[likely]]
            {
                add_delimeter();
                add_to_title(prefix);
                add_to_title(type_list[it->second.type]);
                not_empty_title = 1;
            }
            else
                end_of_title();
            type_count[it->second.type] = 0;
        }
        if (type_count[it->second.type] == 1)
        {
            piece_t name = it->second.name;
            if (title.size >= need_size + name.size) [[likely]]
            {
                add_delimeter();
                add_to_title(it->second.short_info.substr(name.position, name.size));
                not_empty_title = 1;
            }
            else
                end_of_title();
        }
    };
    for (id_or_group_t id_or_group : ids)
    {
        if (id_or_group.type == id_or_group_t::id)
            long_ship_info(id_or_group.value);
        else
        {
            std::unordered_map <int, std::vector <int> > ::iterator it = ship_list_in_type.find(id_or_group.value);
            if (it == ship_list_in_type.end()) [[unlikely]]
                continue;
            for (int id : it->second)
                long_ship_info(id);
        }
    }
}


