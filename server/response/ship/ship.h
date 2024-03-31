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
    static const constexpr std::string_view new_line = "<br>\n";
    static const constexpr std::string_view shift = "&emsp;";
    
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
            "\"></a><br>",
            "</li>\n"
        },
        {
            "<ul>",
            "</ul><br>\n"
        }
    };
    
    static const constexpr std::string_view compare_ships__view_pictures = 
    "<script>\n"
        "function str_to_date (str)\n"
        "{\n"
            "var pattern = /(\\d{2})\\.(\\d{2})\\.(\\d{4})/;\n"
            "return new Date(str.replace(pattern, \"$3-$2-$1\"));\n"
        "}\n"
        
        "var detail_ships_to_compare = new Set();\n"
        "function toggle_to_detail_compare (event)\n"
        "{\n"
            "if (event.target.checked)\n"
                "detail_ships_to_compare.add(event.target);\n"
            "else\n"
                "detail_ships_to_compare.delete(event.target);\n"
            "var request = [];\n"
            "for (item of detail_ships_to_compare)\n"
            "{\n"
                "var date = item.getAttribute(\"date\");\n"
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
            "var link = \"/ship/armament?\";\n"
            "var first = true;\n"
            "for (request_item of request)\n"
            "{\n"
                "if (!first)\n"
                    "link += \"&\";\n"
                "link += \"ship=\" + request_item.ship_id + \"&date=\" + request_item.date_str;\n"
                "first = false;\n"
            "}\n"
            "var compare_ships_buttons = document.getElementById(\"detail_compare_ships_button\");\n"
            "compare_ships_buttons.setAttribute(\"href\", link);\n"
        "}\n"
        
        "var picture_list = [];\n"
        "var picture_index = 0;\n"
        "function view_pictures (event)\n"
        "{\n"
            "picture_list = [];\n"
            "picture_index = 0;\n"
            "for (picture of event.target.nextElementSibling.childNodes)\n"
            "{\n"
                "if (picture.tagName != \"LI\" || picture.childNodes.length < 3)\n"
                    "continue;\n" // text that just new line, skip it
                "var link = picture.childNodes[0];\n"
                "var description = picture.childNodes[2];\n"
                "var picture_link = link.getAttribute(\"href\");"
                "picture_list.push({\n"
                    "picture:     picture_link,\n"
                    "description: description.textContent,\n"
                "});\n"
            "}\n"
            "display_picture();\n"
            "var picture_viewer = document.getElementById(\"picture_viewer\");\n"
            "picture_viewer.style.display = \"flex\";\n"
            "document.addEventListener(\"keydown\", keyboard_control);\n"
            "document.addEventListener(\"touchstart\", touch_start);\n"
            "document.addEventListener(\"touchmove\", touch_move);\n"
            "document.addEventListener(\"touchend\", touch_end);\n"
        "}\n"
        
        "function display_picture ()\n"
        "{\n"
            "var image_src = \"\";\n"
            "var description = \"\";\n"
            "if (picture_list.length != 0)\n"
            "{\n"
                "image_src = picture_list[picture_index].picture;\n"
                "description = picture_list[picture_index].description;\n"
            "}\n"
            "var picture_view = document.getElementById(\"picture_view\");\n"
            "picture_view.setAttribute(\"src\", image_src);\n"
            "var picture_description = document.getElementById(\"picture_description\");\n"
            "picture_description.textContent = description;\n"
        "}\n"
        "function keyboard_control (event)\n"
        "{\n"
            "if (event.key === \"ArrowLeft\")\n"
                "prev_picture();\n"
            "else if (event.key === \"ArrowRight\")\n"
                "next_picture();\n"
            "else if (event.key === \"Escape\")\n"
                "clear_picture();\n"
        "}\n"
        "var touch_start_x;\n"
        "var touch_start_y;\n"
        "var touch_single;\n"
        "var touch_time;\n"
        "function touch_start (event)\n"
        "{\n"
            "var picture_viewer = document.getElementById(\"picture_viewer\");\n"
            "touch_start_x = event.changedTouches[0].clientX;\n"
            "touch_start_y = event.changedTouches[0].clientY;\n"
            "touch_single = event.changedTouches.length == 1 && touch_start_x >= picture_viewer.getBoundingClientRect().left;\n"
            "touch_time = event.timeStamp;\n"
        "}\n"
        "function touch_move (event)\n"
        "{\n"
            "touch_single &= event.changedTouches.length == 1;\n"
        "}\n"
        "function touch_end (event)\n"
        "{\n"
            "touch_single &= event.changedTouches.length == 1;\n"
            "if (!touch_single || event.timeStamp - touch_time > 500)\n"
                "return;\n"
            "var touch_end_x = event.changedTouches[0].clientX;\n"
            "var touch_end_y = event.changedTouches[0].clientY;\n"
            "if (Math.abs(touch_end_x - touch_start_x) > 2 * Math.abs(touch_end_y - touch_start_y))\n"
            "{\n"
                "if (touch_end_x - touch_start_x > 0)\n"
                    "prev_picture();\n"
                "else\n"
                    "next_picture();\n"
            "}\n"
        "}\n"
        "function next_picture ()\n"
        "{\n"
            "if (picture_list.length == 0)\n"
                "return;\n"
            "picture_index = (picture_index + 1) % picture_list.length;\n"
            "display_picture();\n"
        "}\n"
        "function prev_picture ()\n"
        "{\n"
            "if (picture_list.length == 0)\n"
                "return;\n"
            "picture_index = (picture_index + picture_list.length - 1) % picture_list.length;\n"
            "display_picture();\n"
        "}\n"
        "function clear_picture ()\n"
        "{\n"
            "picture_list = [];\n"
            "picture_index = 0;\n"
            "var picture_viewer = document.getElementById(\"picture_viewer\");\n"
            "document.removeEventListener(\"touchstart\", touch_start);\n"
            "document.removeEventListener(\"touchmove\", touch_move);\n"
            "document.removeEventListener(\"touchend\", touch_end);\n"
            "document.removeEventListener(\"keydown\", keyboard_control);\n"
            "picture_viewer.style.display = \"none\";\n"
            "display_picture();\n"
        "}\n"
    "</script>\n"
    "<style>\n"
        ".picture_viewer {\n"
            "position: fixed;\n"
            "left:   calc(170pt + 10px);\n"
            "right:  0px;\n"
            "top:    0px;\n"
            "bottom: 0px;\n"
            "display: none;\n"
            "background-color: #f5f5f5;\n"
            "justify-content: center;\n"
            "align-items: center;\n"
            "min-width: 200px;\n"
            "min-height: 200px;\n"
        "}\n"
        ".picture_viewer img {\n"
            "max-width: calc(100% - 10px - 50pt);\n"
            "max-height: calc(100% - 140px);\n"
        "}\n"
        ".picture_viewer p {\n"
            "font-size: 20px;\n"
            "position: fixed;\n"
            "bottom: 20px;\n"
        "}\n"
        ".picture_viewer button {\n"
            "position: fixed;\n"
            "text-shadow: 1px 0px 3px white, -1px 0px 3px white, 0px 1px 3px white, 0px -1px 3px white;\n"
            "font-size: 30px;\n"
            "background-color: transparent;\n"
            "border: none;\n"
            "cursor: pointer;\n"
        "}\n"
        ".picture_viewer button:hover {\n"
            "color: #ff5555;\n"
        "}\n"
        ".close_button {\n"
            "right: 3px;\n"
            "top: 3px;\n"
        "}\n"
        ".prev_button {\n"
            "left: calc(170pt + 10px + 3px);\n"
            "top: calc(50vh - 75px);\n"
            "min-height: 150px;\n"
        "}\n"
        ".next_button {\n"
            "right: 3px;\n"
            "top: calc(50vh - 75px);\n"
            "min-height: 150px;\n"
        "}\n"
        ".view_pictures {\n"
            "background-color: #777777;\n"
            "color: white;\n"
            "padding: 6px;\n"
            "border: none;\n"
            "outline: none;\n"
            "cursor: pointer;\n"
        "}\n"
        ".view_pictures:hover { background-color: #ff5555; }\n"
        ".view_pictures:focus { background-color: #6666ff; }\n"
    "</style>\n"
    "<div class=\"menu_link\"><a id=\"detail_compare_ships_button\" href=\"/ship/armament?\">сравнение корабликов</a></div>\n"
    "<div id=\"picture_viewer\" class=\"picture_viewer\">\n"
        "<img src=\"\" id=\"picture_view\">\n"
        "<button class=\"close_button\" onclick=clear_picture(event)> x </button>\n"
        "<p id=\"picture_description\"> </p>\n"
        "<button class=\"prev_button\" onclick=prev_picture(event)> < </button>\n"
        "<button class=\"next_button\" onclick=next_picture(event)> > </button>\n"
    "</div>\n";
};



#endif

