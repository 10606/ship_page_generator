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
    
    item new_class = {"</div></div>\n\
                       <button type = \"button\" class = \"collapsible\">\n", 
                      "</button><br>\n\
                       <div class = \"content\"><div>\n"};
    
    item new_type = {"</div>\n\
                      <button type = \"button\" class = \"collapsible\">\n", 
                     "</button><br>\n\
                      <div class = \"content\">\n"};
    
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

