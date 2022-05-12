#include "ship.h"

#include "nested_segments.h"
#include "ship_requests.h"
#include "ship_event.h"
#include "ship_info.h"
#include "date_to_str.h"
#include "parse_query.h"


void add_modernizations
(
    std::string & answer, 
    std::vector <ship_requests::ship_event_t::event_lt_descr> const & events,
    std::vector <size_t> const & index_mapping,
    std::vector <segment> segments
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
    
    for (std::vector <segment> :: iterator it = segments.begin(); it != part; ++it)
        if (it->begin)
            answer.append("&date=")
                  .append(to_string(*it->end));
}


struct add_event
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
        visited[vertex] = 0;
        size_t index = index_mapping[vertex];
        
        for (size_t i = 0; i != shift; ++i)
            answer.append(ship::shift);
        if (events[index].date_from)
            answer.append(to_string(*events[index].date_from));
        answer.append(" - ");
        if (events[index].date_to)
            answer.append(to_string(*events[index].date_to));
        answer.append(" ");
        if (events[index].class_ru)
            answer.append(*events[index].class_ru)
                  .append(": ");
        if (events[index].description)
            answer.append(*events[index].description);
        answer.append(ship::new_line);
        
        for (size_t next : graph[vertex])
            visit(next, shift + 1);
    }
    
    void operator () ()
    {
        std::vector <size_t> order(graph.size());
        for (size_t i = 0; i != graph.size(); ++i)
            order[i] = i;
        
        auto comparator = 
            [this] (size_t a, size_t b) -> bool 
            { 
                return std::is_lt(compare_null_last(events[index_mapping[a]].date_from, 
                                                    events[index_mapping[b]].date_from)); 
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
};


void add_general_info
(
    std::string & answer, 
    std::string & modernization_link, 
    ship_requests::ship_info_t::list const & info
)
{
    if (info.ship_ru)
        answer.append(*info.ship_ru)
              .append(" ");
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
    answer.append(ship::new_line);
    
    if (info.commissioned)
    {
        std::string commissioned_str = to_string(*info.commissioned);
        answer.append(commissioned_str);
        modernization_link.append("&date=")
                          .append(std::move(commissioned_str));
    }
    answer.append(" -> ");
    if (info.sunk_date)
    {
        answer.append(to_string(*info.sunk_date));
        if (info.sunk_reason)
        {
            answer.append(" (")
                  .append(*info.sunk_reason)
                  .append(") ");
        }
    }
    answer.append(ship::new_line);
}


ship::ship (ship_requests * _database, ship_armament & _armament) :
    database(_database),
    armament(_armament)
{
    std::vector <ship_requests::ship_event_t::event_lt_descr> events = 
        database->ship_event.get_event_lt_descr();
    
    std::vector <ship_requests::ship_info_t::list> list =
        database->ship_info.get_list();
    
    std::unordered_map <int, std::vector <segment> > ship_to_segment;
    std::unordered_map <int, std::vector <size_t> > index_mapping; // 0..size -> event index
    typedef ship_requests::ship_info_t::list list_t;
    std::unordered_map <int, list_t> ship_info;
    
    for (size_t i = 0; i != events.size(); ++i)
    {
        size_t index = ship_to_segment[events[i].ship_id].size();
        ship_to_segment[events[i].ship_id].emplace_back(events[i].date_from, events[i].date_to, index);
        index_mapping[events[i].ship_id].push_back(i);
    }

    for (size_t i = 0; i != list.size(); ++i)
        ship_info.insert({list[i].ship_id, std::move(list[i])});

    for (auto & info : ship_info)
    {
        int ship_id = info.first;
        
        response_t answer;
        answer.armament_link = std::string(query_template);
        answer.armament_link.append(std::to_string(ship_id));
        
        // general info
        {
            add_general_info(answer.begin, answer.armament_link, info.second);
        }

        auto & ship_data = ship_to_segment[ship_id];
        std::vector <size_t> const & index_mapping_value = index_mapping[ship_id];
        
        // modernizations link
        {
            add_modernizations(answer.armament_link, events, index_mapping_value, ship_data);
        }
        
        // add events
        {
            std::vector <std::vector <size_t> > nested = nested_segments(std::move(ship_data));
            add_event event_adder(answer.begin, events, std::move(nested), index_mapping_value);
            event_adder();
        }
        
        answer.begin.append(link.begin)
                    .append(answer.armament_link)
                    .append(link.end);
        answer.end.append(new_line)
                  .append(new_line);
        modernizations.insert({ship_id, std::move(answer)});
    }
}


void ship::response (std::string & answer, std::string_view query)
{
    std::vector <int> ids = parse_query__id(query);
    
    for (int id : ids)
    {
        std::unordered_map <int, response_t> :: iterator it = modernizations.find(id);
        if (it == modernizations.end())
            continue;
        answer.append(it->second.begin);
        armament.response(answer, it->second.armament_link);
        answer.append(it->second.end);
    }
}


