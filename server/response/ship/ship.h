#ifndef SHIP_H
#define SHIP_H

#include "ship_event.h"
#include "response_ship_armament.h"
#include "simple_string.h"
#include "html_template.h"
#include <map>


struct ship
{
    ship (ship_requests * _database, ship_armament & _armament);
    
    // http://127.0.0.1:8080/ship?id=0&id=1&id=2&id=3
    void response (simple_string & answer, std::string_view query, piece_t title);
    
private:
    ship_armament & armament;
    
    struct response_t
    {
        std::string short_info;
        piece_t name; // to short_info
        size_t type;

        std::string begin;
        std::string armament_link;
        std::string end;
    };
    
    std::unordered_map <int, response_t> modernizations;
    std::vector <std::string> type_list;
    std::unordered_map <int, std::vector <int> > ship_list_in_type;
    
    struct add_event;
    
    static void add_general_info
    (
        std::string & answer, 
        std::string & modernization_link, 
        ship_requests::ship_info_t::list const & info
    );

    static void add_short_info
    (
        response_t & answer, 
        ship_requests::ship_info_t::list const & info
    );
    
    static const constexpr std::string_view query_template = "ship=";
    static const constexpr html_template link = {"<a href = \"/ship/armament?", "\">вооружение</a>"};
    static const constexpr std::string_view new_line = "<br>\n";
    static const constexpr std::string_view shift = "&emsp;";
    
    static const constexpr pictures_template pictures =
    {
        {
            "<li><a href = \"/pictures/ship/",
            "\"><img src = \"/pictures_small/ship/",
            "\"></a><br>",
            "</li>"
        },
        {
            "<ul>",
            "</ul><br>"
        }
    };
    
    static const constexpr std::string_view detail_compare_ships = 
    "<script>\n"
        "function str_to_date (str) {\n"
            "var pattern = /(\\d{2})\\.(\\d{2})\\.(\\d{4})/;\n"
            "return new Date(str.replace(pattern,'$3-$2-$1'));\n"
        "}\n"
        "function detail_compare_ships () {\n"
            "var request = [];\n"
            "var main = document.getElementsByClassName(\"main\");\n"
            "for (var i = 0; i < main.length; i++) {\n"
                "var checks = main[i].getElementsByClassName(\"checkbox\");\n"
                "for (j = 0; j < checks.length; j++) {\n"
                    "if (checks[j].checked) {\n"
                        "var date = checks[j].getAttribute(\"date\");\n"
                        "request.push({\n" 
                            "ship_id:   checks[j].getAttribute(\"ship_id\"),\n"
                            "date_str:  date,\n"
                            "date:      str_to_date(date),\n"
                        "});\n"
                    "}\n"
                "}\n"
            "}\n"
            "request.sort(\n"
                "function (a, b) {\n"
                    "if (a.date < b.date)\n"
                        "return -1;\n"
                    "if (a.date > b.date)\n"
                        "return 1;\n"
                    "return a.ship_id - b.ship_id;\n"
                "}\n"
            ");\n"
            "var link = \"/ship/armament?\";\n"
            "var first = true;\n"
            "for (var i = 0; i < request.length; i++) {\n"
                "if (!first) {\n"
                    "link += \"&\";\n"
                "}\n"
                "link += \"ship=\" + request[i].ship_id + \"&date=\" + request[i].date_str;\n"
                "first = false;\n"
            "}\n"
            "window.location.href = link;\n"
            "return link;\n"
        "}\n"
    "</script>\n"
    "<div class = \"menu_link\"><a href = \"#\" onclick = \"detail_compare_ships()\">сравнение корабликов</a></div>\n";
};



#endif

