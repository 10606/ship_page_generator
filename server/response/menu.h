#ifndef MENU_H
#define MENU_H

#include <string_view>
#include <vector>
#include "ship_requests.h"
#include "simple_string.h"
#include "html_template.h"


struct menu_item_template
{
    menu_item_template (ship_requests * database) :
        something_needed(generate_aircraft_links(database, around, link_template)),
        all{menu_begin, something_needed, menu_end}
    {}

private:
    std::string something_needed;
    static std::string generate_aircraft_links (ship_requests * database, html_template around, html_template_3 link);

    static std::string_view menu_begin;
    static html_template around;
    static html_template_3 link_template;
    static std::string_view menu_end;

public:
    html_template_3 all;
    
    html_template_3 new_class = {"<button type=\"button\" class=\"collapsible\">\n", 
                                 "<ship-cnt>(",
                                 ")</ship-cnt></button><br><div class=\"content\">\n"};
    
    html_template new_type_link = {"<button type=\"button\" class=\"collapsible\">\n<a href=\"/ship?type_id=", 
                                   "\">"};
    html_template_3 new_type = {"", 
                                "</a><ship-cnt>(",
                                ")</ship-cnt></button><br><div class=\"content\">\n"};
    
    std::string_view close_type = "</div>";
    std::string_view close_class = "</div>";

    html_template new_ship = {"", "<br>"};
    html_template_3 new_ship_link = {"<a href=\"/ship?id=", "\">", "</a>"};
    html_template checkbox = {"<input type=\"checkbox\" ship_id=\"", "\"></input>"};
};

struct menu
{
    menu (ship_requests * database) :
        menu_item(database),
        cache(response_impl(database))
    {}
    
    void response (simple_string & answer, std::string_view request);
    
private:
    struct cache_t
    {
        std::string begin;
        std::string end;
    };

    cache_t response_impl (ship_requests * database);
    
    menu_item_template menu_item;
    cache_t cache;
};

#endif

