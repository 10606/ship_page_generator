#include "menu.h"

#include <vector>
#include <map>
#include <queue>

#include "ship_info.h"
#include "date_to_str.h"
#include "search.h"


struct cur_type_t
{
    void reset ()
    {
        id.reset();
        type_descr.clear();
        link.clear();
        ships_in_type.clear();
        min_date.reset();
    }

    std::optional <int> id;
    std::string type_descr;
    std::string link;
    std::string ships_in_type;
    std::optional <std::chrono::year_month_day> min_date;
};

struct cur_class_t
{
    void reset (int _id)
    {
        id = _id;
        types.clear();
    }

    void sort ()
    {
        static const auto comparator = [] (cur_type_t const & a, cur_type_t const & b) -> bool
        {
            if (a.min_date && b.min_date)
            {
                if (a.min_date == b.min_date)
                    return a.id < b.id;
                else
                    return a.min_date < b.min_date;
            }
            else if (a.min_date)
                return 1;
            else if (b.min_date)
                return 0;
            else
                return a.id < b.id;
        };
        
        std::sort(types.begin(), types.end(), comparator);
    }
    
    std::optional <int> id;
    std::string class_descr;
    std::vector <cur_type_t> types;
    std::string_view close_class;
};

struct inserter_t
{
    inserter_t 
    (
        std::map <int, std::vector <int> > const & _classes_graph,
        std::map <int, cur_class_t> _classes,
        std::string & _answer
    ) :
        classes_graph(_classes_graph),
        classes(_classes),
        answer(_answer)
    {}

    void operator () (int class_id)
    {
        std::map <int, cur_class_t> :: iterator it_class = classes.find(class_id);
        if (it_class == classes.end())
            return;
        
        answer.append(it_class->second.class_descr);
        for (auto const & cur_type : it_class->second.types)
            answer.append(cur_type.link)
                  .append(cur_type.type_descr)
                  .append(cur_type.ships_in_type);
        
        std::map <int, std::vector <int> > :: const_iterator it_graph = classes_graph.find(class_id);
        
        // one column...
        answer.append(it_class->second.close_class);
        
        if (it_graph != classes_graph.end())
        {
            for (int child : it_graph->second)
                operator () (child);
        }
        
        // tree like
        //answer.append(it_class->second.close_class);
    };
    
    std::map <int, std::vector <int> > const & classes_graph;
    std::map <int, cur_class_t> classes;
    std::string & answer;
};


menu::cache_t menu::response_impl (ship_requests * database)
{
    try
    {
        // graph for classes
        std::vector <int> classes_root;
        std::map <int, std::vector <int> > classes_graph;
        
        std::vector <ship_requests::ship_info_t::classes> class_list =
                database->ship_info.get_classes("order by (id)");
        for (auto const & cur_class : class_list)
        {
            if (!cur_class.parent_id)
                classes_root.push_back(cur_class.class_id);
            else
                classes_graph[*cur_class.parent_id].push_back(cur_class.class_id);
        }
        
        

        std::vector <ship_requests::ship_info_t::list> ships =
                database->ship_info.get_list("order by (ship_list.class_id, ship_list.type_id, \
                                                        commissioned, ship_list.name_ru,  ship_list.id)");
     
        std::map <int, cur_class_t> classes;
        cur_class_t cur_class;
        cur_type_t cur_type;
        
        cur_class.close_class = menu_item.close_class;
        
        auto add_type = 
        [
            close_type = menu_item.close_type, 
            new_type_link = menu_item.new_type_link,
            &cur_class, 
            &cur_type
        ] () -> void
        {
            cur_type.link.append(new_type_link.end);
            cur_type.ships_in_type.append(close_type);
            cur_class.types.push_back(std::move(cur_type));
            cur_type.reset();
        };

        std::queue <size_t> cnt_ships_in_class;
        std::queue <size_t> cnt_ships_in_type;
        for (size_t i = 0; i != ships.size(); ++i)
        {
            if (cnt_ships_in_class.empty() ||
                ships[i - 1].class_id != ships[i].class_id)
                cnt_ships_in_class.push(1);
            else
                cnt_ships_in_class.back()++;

            if (cnt_ships_in_type.empty() ||
                ships[i - 1].type_id != ships[i].type_id)
                cnt_ships_in_type.push(1);
            else
                cnt_ships_in_type.back()++;
        }
        
        for (size_t i = 0; i != ships.size(); ++i)
        {
            auto const & ship = ships[i];
            
            if (!cur_class.id || *cur_class.id != ship.class_id)
            {
                // new class
                if (cur_class.id)
                {
                    add_type();
                    cur_class.sort();
                    classes.insert({*cur_class.id, std::move(cur_class)});
                }
                cur_class.reset(ship.class_id);
                
                cur_class.class_descr.append(menu_item.new_class.begin)
                                     .append(ship.class_ru.value_or(" -- "))
                                     .append(menu_item.new_class.middle)
                                     .append(std::to_string(cnt_ships_in_class.front()))
                                     .append(menu_item.new_class.end);
                cnt_ships_in_class.pop();
            }
            
            if (!cur_type.id || *cur_type.id != ship.type_id)
            {
                // new type
                if (cur_type.id)
                    add_type();
                cur_type.id = ship.type_id;
                
                
                cur_type.type_descr.append(menu_item.new_type.begin);
                if ((i + 1 == ships.size() || ships[i].type_id != ships[i + 1].type_id) &&
                    ship.ship_ru &&
                    (!ship.type_ru || *ship.ship_ru != *ship.type_ru)) // one ship in type with another name
                    cur_type.type_descr.append(*ship.ship_ru);
                else
                    cur_type.type_descr.append(ship.type_ru.value_or(" -- "));

                if (ship.commissioned)
                    cur_type.type_descr.append(" ")
                                       .append(std::to_string(static_cast <int> (ship.commissioned->year())));
                
                cur_type.type_descr.append(menu_item.new_type.middle)
                                   .append(std::to_string(cnt_ships_in_type.front()))
                                   .append(menu_item.new_type.end);
                cnt_ships_in_type.pop();
            }
            
            {
                // new ship
                if (ship.commissioned)
                    if (!cur_type.min_date || *ship.commissioned < *cur_type.min_date)
                        cur_type.min_date = ship.commissioned;
                
                cur_type.ships_in_type.append(menu_item.new_ship.begin)
                                      .append(menu_item.new_ship_link.begin)
                                      .append(std::to_string(ship.ship_id))
                                      .append(menu_item.new_ship_link.end)
                                      .append(ship.ship_ru.value_or(" --- "))
                                      .append(menu_item.new_ship.end);
                
                if (cur_type.link.empty())
                    cur_type.link.append(menu_item.new_type_link.begin);
                else
                    cur_type.link.append("&id=");
                cur_type.link.append(std::to_string(ship.ship_id));
            }
        }
        add_type();
        cur_class.sort();
        classes.insert({*cur_class.id, std::move(cur_class)});
        
        cache_t answer{std::string(menu_item.all.begin), std::string(menu_item.all.middle)};
        inserter_t inserter(classes_graph, classes, answer.end);
        for (int root : classes_root)
            inserter(root);
        answer.end.append(menu_item.all.end);
        
        return answer;
    }
    catch (...)
    {
        return cache_t();
    }
}


void menu::response (simple_string & answer, std::string_view request)
{
    answer.append(cache.begin);
    answer.append(search::get_search_parameter(request));
    answer.append(cache.end);
}


