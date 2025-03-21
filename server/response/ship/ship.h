#ifndef SHIP_H
#define SHIP_H

#include "response.h"
#include "ship_event.h"
#include "response_ship_armament.h"
#include "simple_string.h"
#include "html_template.h"
#include "nested_segments.h"
#include <map>


struct ship : response_base
{
    ship (ship_requests & database, ship_armament & _armament);
    
    // http://127.0.0.1:8080/ship?id=0&id=1&id=2&id=3
    virtual void response (simple_string & answer, std::string_view query, piece_t title) override;
    
    virtual std::string_view additional_in_menu () const override
    {
        return  "<div class=\"menu_link tooltip\">\n"
                    "<a id=\"detail_compare_ships_button\" href=\"/ship/armament?\">детальное сравнение</a>\n"
                    "<span class=\"tooltiptext\">отмеченные галочками в заголовках таблиц</span>\n"
                "</div>\n";
    }
    
private:
    ship_armament & armament;
    
    struct response_t
    {
        std::string short_info;
        piece_t name; // to short_info
        size_t type;

        std::string begin;
        std::vector <std::pair <int, std::chrono::year_month_day> > ship_year;
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
        std::vector <std::pair <int, std::chrono::year_month_day> > & ship_year,
        ship_requests::ship_info_t::list const & info
    );

    static void add_short_info
    (
        response_t & answer, 
        ship_requests::ship_info_t::list const & info
    );
    
    static void add_modernizations
    (
        std::string & answer, 
        std::vector <std::pair <int, std::chrono::year_month_day> > & ship_year,
        ship_requests::ship_info_t::list const & info,
        std::vector <ship_requests::ship_event_t::event_lt_descr> const & events,
        std::vector <size_t> const & index_mapping,
        std::vector <segment> segments
    );
    
    static const constexpr std::string_view query_template = "ship=";
    static const constexpr std::string_view new_line = "<br>\n";
    static const constexpr std::string_view shift = "&emsp;&emsp;";
    
    static const constexpr html_template_3 name = {"<b-t id=\"id_", "\">", "</b-t> "};
    static const constexpr html_template link = {"<a href=\"/ship/armament?", "\">вооружение</a>"};
    static const constexpr html_template text = {"<span>", "</span>"};
    static const constexpr html_template bold_text = {"<b-t>", "</b-t>"};
    static const constexpr html_template row = {"<tr><td>", "</td></tr>\n"};
    
    static const constexpr pictures_template pictures =
    {
        {
            "<li><a href=\"/pictures/ship/",
            "\"><img src=\"/pictures_small/ship/",
        },
    };

    static const constexpr std::string_view compare_ships = 
    "<script>\n"
        "function str_to_date (str)\n"
        "{\n"
            "let pattern = /(\\d{2})\\.(\\d{2})\\.(\\d{4})/;\n"
            "return new Date(str.replace(pattern, \"$3-$2-$1\"));\n"
        "}\n"
        
        "var detail_ships_to_compare = new Set();\n"
        "function toggle_to_detail_compare (event)\n"
        "{\n"
            "if (event.target.checked)\n"
                "detail_ships_to_compare.add(event.target);\n"
            "else\n"
                "detail_ships_to_compare.delete(event.target);\n"
            "let request = [];\n"
            "for (item of detail_ships_to_compare)\n"
            "{\n"
                "let date = item.getAttribute(\"date\");\n"
                "request.push({\n" 
                    "ship_id:   item.getAttribute(\"ship\").toString(),\n"
                    "date_str:  date,\n"
                    "date:      str_to_date(date),\n"
                "});\n"
            "}\n"
            "request.sort(\n"
                "function (a, b)\n"
                "{\n"
                    "if (a.date < b.date)\n"
                        "return -1;\n"
                    "if (a.date > b.date)\n"
                        "return 1;\n"
                    "return a.ship_id - b.ship_id;\n"
                "}\n"
            ");\n"
            "let link = \"/ship/armament?\";\n"
            "let first = true;\n"
            "for (request_item of request)\n"
            "{\n"
                "if (!first)\n"
                    "link += \"&\";\n"
                "link += \"ship=\" + request_item.ship_id + \"&date=\" + request_item.date_str;\n"
                "first = false;\n"
            "}\n"
            "let compare_ships_button = document.getElementById(\"detail_compare_ships_button\");\n"
            "compare_ships_button.setAttribute(\"href\", link);\n"
        "}\n"
    "</script>\n";
};



#endif

