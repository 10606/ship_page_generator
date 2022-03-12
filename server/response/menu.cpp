#include "menu.h"

#include <vector>
#include <map>

#include "ship_info.h"
#include "date_to_str.h"


menu_item_template::item const menu_item_template::all_template =
{
    "<div class = \"menu\"> \
        <style> \
            .collapsible { \
              background-color: #777777; \
              color: white; \
              cursor: pointer; \
              padding: 6px; \
              border: none; \
              text-align: left; \
              outline: none; \
              font-size: 15px; \
            } \
            \
            .active, .collapsible:hover { \
              background-color: #555555; \
            } \
            \
            .content { \
              padding: 0px 20px; \
              display: none; \
              overflow: hidden; \
              background-color: #ffffff; \
            } \
        </style>\
        <div><div>",
        
        "</div></div>\
        <script>\
            var coll = document.getElementsByClassName(\"collapsible\");\
            var i;\
            \
            for (i = 0; i < coll.length; i++) {\
                coll[i].addEventListener(\"click\", function() {\
                    this.classList.toggle(\"active\");\
                    var content = this.nextElementSibling.nextElementSibling;\
                    if (content.style.display === \"block\") {\
                        content.style.display = \"none\";\
                    } else {\
                        content.style.display = \"block\";\
                    }\
                });\
            }\
        </script>\
    </div>"
};


struct cur_type_t
{
    void reset ()
    {
        id.reset();
        type_descr.clear();
        min_date.reset();
    }

    std::optional <int> id;
    std::string type_descr;
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
            answer.append(cur_type.type_descr);
        
        std::map <int, std::vector <int> > :: const_iterator it_graph = classes_graph.find(class_id);
        if (it_graph == classes_graph.end())
            return;
        for (int child : it_graph->second)
            operator () (child);
    };
    
    std::map <int, std::vector <int> > const & classes_graph;
    std::map <int, cur_class_t> classes;
    std::string & answer;
};


std::string menu::response ()
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
    
    auto add_type = [&cur_class, &cur_type] () -> void
    {
        cur_class.types.push_back(std::move(cur_type));
        cur_type.reset();
    };
    
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
                                 .append(menu_item.new_class.end);
        }
        
        if (!cur_type.id || *cur_type.id != ship.type_id)
        {
            // new type
            if (cur_type.id)
                add_type();
            cur_type.id = ship.type_id;
            
            cur_type.type_descr.append(menu_item.new_type.begin)
                               .append(ship.type_ru.value_or(" -- "));
            
            if (i + 1 != ships.size() && 
                ships[i].type_id != ships[i + 1].type_id &&
                ship.ship_ru &&
                (!ship.type_ru || *ship.ship_ru != *ship.type_ru))
                cur_type.type_descr.append(" (").append(*ship.ship_ru).append(")");
            
            cur_type.type_descr.append(menu_item.new_type.end);
        }
        
        {
            // new ship
            if (ship.commissioned)
                if (!cur_type.min_date || *cur_type.min_date < *ship.commissioned)
                    cur_type.min_date = ship.commissioned;
            
            cur_type.type_descr.append(menu_item.new_ship.begin)
                               .append(ship.ship_ru.value_or(" --- "))
                               .append(menu_item.new_ship.end);
        }
    }
    
    std::string answer(menu_item.all.begin);
    inserter_t inserter(classes_graph, classes, answer);
    for (int root : classes_root)
        inserter(root);
    answer.append(menu_item.all.end);
    
    return answer;
};

