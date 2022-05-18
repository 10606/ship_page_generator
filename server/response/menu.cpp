#include "menu.h"

#include <vector>
#include <map>

#include "ship_info.h"
#include "date_to_str.h"


menu_item_template::item const menu_item_template::all_template =
{
    "<div class = \"menu\"> \n\
        <style> \n\
            .collapsible { \n\
              background-color: #777777; \n\
              color: white; \n\
              cursor: pointer; \n\
              padding: 6px; \n\
              border: none; \n\
              text-align: left; \n\
              outline: none; \n\
              font-size: 15px; \n\
            } \n\
            \n\
            .collapsible a { \n\
              color: #ffffff; \n\
            } \n\
            \n\
            .active, .collapsible:hover { \n\
              background-color: #555555; \n\
            } \n\
            \n\
            .content { \n\
              padding: 0px 20px; \n\
              display: none; \n\
              overflow: hidden; \n\
              background-color: #ffffff; \n\
            } \n\
        </style>\n",
        
        "<script>\n\
            var coll = document.getElementsByClassName(\"collapsible\");\n\
            var i;\n\
            \n\
            for (i = 0; i < coll.length; i++) {\n\
                coll[i].addEventListener(\"click\", function() {\n\
                    this.classList.toggle(\"active\");\n\
                    var content = this.nextElementSibling.nextElementSibling;\n\
                    if (content.style.display === \"block\") {\n\
                        content.style.display = \"none\";\n\
                    } else {\n\
                        content.style.display = \"block\";\n\
                    }\n\
                });\n\
            }\n\
        </script>\n\
    </div>\n"
};


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


std::string menu::response_impl ()
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
                    (!ship.type_ru || *ship.ship_ru != *ship.type_ru)) // one ship in type with another name
                    cur_type.type_descr.append(" (").append(*ship.ship_ru).append(")");
                
                if (ship.commissioned)
                    cur_type.type_descr.append(" ")
                                       .append(std::to_string(static_cast <int> (ship.commissioned->year())));
                
                cur_type.type_descr.append(menu_item.new_type.end);
            }
            
            {
                // new ship
                if (ship.commissioned)
                    if (!cur_type.min_date || *cur_type.min_date < *ship.commissioned)
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
        
        std::string answer(menu_item.all.begin);
        inserter_t inserter(classes_graph, classes, answer);
        for (int root : classes_root)
            inserter(root);
        answer.append(menu_item.all.end);
        
        return answer;
    }
    catch (...)
    {
        return "";
    }
}


void menu::response (std::string & answer)
{
    static std::string cache = response_impl();
    answer.append(cache);
}


