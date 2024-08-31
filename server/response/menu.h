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
        aircraft_links(generate_aircraft_links(database, around, link_template))
    {}

private:
    static std::string generate_aircraft_links (ship_requests * database, html_template around, html_template_3 link);

    static html_template around;
    static html_template_3 link_template;

public:
    static std::string_view menu_begin;
    std::string aircraft_links;
    static html_template menu_end;
    
    html_template_3 new_class = {"<button type=\"button\"class=\"collapsible\">", 
                                 "<ship-cnt>(",
                                 ")</ship-cnt></button><br><div class=\"content\">\n"};
    
    html_template_3 new_type_link = {"<button type=\"button\"class=\"collapsible\"><a href=\"/ship?type_id=", 
                                     "\">",
                                     "</a>"};
    html_template new_type = {"<ship-cnt>(",
                              ")</ship-cnt></button><br><div class=\"content\">\n"};
    
    std::string_view close_type = "</div>\n";
    std::string_view close_class = "</div>\n";

    html_template new_ship = {"", "<br>"};
    html_template_3 new_ship_link = {"<a href=\"/ship?id=", "\">", "</a>"};
    html_template checkbox = {"<input type=\"checkbox\"ship=", "></input>"};
};

struct menu
{
    menu (ship_requests * database) :
        menu_item(database),
        cache(response_impl(database))
    {}
    
    void response (simple_string & answer, std::string_view request, std::string_view additional_in_menu = {});
    
private:
    struct cache_t
    {
        std::string_view begin;
        std::string middle;
        std::string_view end;
    };

    cache_t response_impl (ship_requests * database);
    
    menu_item_template menu_item;
    cache_t cache;
};

#endif

