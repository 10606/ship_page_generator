#ifndef MENU_H
#define MENU_H

#include <string_view>
#include <vector>
#include "ship_requests.h"


struct menu_item_template
{
    struct item
    {
        std::string_view begin;
        std::string_view end;
    };
    
    item all = all_template;
    
    item new_class = {"</div></div>\
                       <button type = \"button\" class = \"collapsible\">", 
                      "</button><br> \
                       <div class = \"content\"><div>"};
    
    item new_type = {"</div>\
                      <button type = \"button\" class = \"collapsible\">", 
                     "</button><br>\
                      <div class = \"content\">"};
    
    item new_ship = {"", "<br>"};
    
private:
    static const item all_template;
};

struct menu
{
    menu (ship_requests * _database, menu_item_template const & _menu_item = menu_item_template()) :
        menu_item(_menu_item),
        database(_database)
    {}
    
    std::string response ();
    
private:
    menu_item_template menu_item;
    ship_requests * database;
};

#endif

