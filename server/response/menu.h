#ifndef MENU_H
#define MENU_H

#include <string_view>
#include <vector>
#include "ship_requests.h"
#include "simple_string.h"
#include "html_template.h"


struct menu_item_template
{
    html_template_3 all = all_template;
    
    html_template new_class = {"<button type = \"button\" class = \"collapsible\">\n", 
                      "</button><br><div class = \"content\">\n"};
    
    html_template new_type_link = {"<button type = \"button\" class = \"collapsible\">\n&nbsp;&nbsp;&nbsp;<a href=\"/ship?id=", 
                          "\">"};
    html_template new_type = {"", 
                     "</a></button><br><div class = \"content\">\n"};
    
    std::string_view close_type = "</div>";
    std::string_view close_class = "</div>";

    html_template new_ship = {"&nbsp;&nbsp;", "</a><br>"};
    html_template new_ship_link = {"<a href=\"/ship?id=", "\">"};
    
private:
    static const html_template_3 all_template;
};

struct menu
{
    menu (ship_requests * database, menu_item_template const & _menu_item = menu_item_template()) :
        menu_item(_menu_item),
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

