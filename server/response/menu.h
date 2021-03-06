#ifndef MENU_H
#define MENU_H

#include <string_view>
#include <vector>
#include "ship_requests.h"
#include "simple_string.h"


struct menu_item_template
{
    struct item
    {
        std::string_view begin;
        std::string_view end;
    };
    
    item all = all_template;
    
    item new_class = {"<button type = \"button\" class = \"collapsible\">\n", 
                      "</button><br><div class = \"content\">\n"};
    
    item new_type_link = {"<button type = \"button\" class = \"collapsible\">\n&nbsp;&nbsp;&nbsp;<a href=\"/ship?id=", 
                          "\">"};
    item new_type = {"", 
                     "</a></button><br><div class = \"content\">\n"};
    
    std::string_view close_type = "</div>";
    std::string_view close_class = "</div>";

    item new_ship = {"&nbsp;&nbsp;", "</a><br>"};
    item new_ship_link = {"<a href=\"/ship?id=", "\">"};
    
private:
    static const item all_template;
};

struct menu
{
    menu (ship_requests * database, menu_item_template const & _menu_item = menu_item_template()) :
        menu_item(_menu_item),
        cache(response_impl(database))
    {}
    
    void response (simple_string & answer);
    
private:
    std::string response_impl (ship_requests * database);
    
    menu_item_template menu_item;
    std::string cache;
};

#endif

