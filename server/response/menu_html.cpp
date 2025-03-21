#include "menu.h"
#include "aircraft_info.h"


std::string menu_item_template::generate_aircraft_links (ship_requests * database, html_template around, html_template_3 link)
{
    typedef ship_requests::aircraft_info_t::classes aircraft_class;
    std::vector <ship_requests::aircraft_info_t::classes> aircraft_classes =
        database->aircraft_info.get_classes("order by id");

    std::string answer = std::string(around.begin);

    for (aircraft_class const & item : aircraft_classes)
    {
        answer.append(link.begin)
              .append(std::to_string(item.class_id))
              .append(link.middle)
              .append(item.class_ru.value_or("---"))
              .append(link.end);
    }

    answer.append(around.end);
    return answer;
}

std::string_view menu_item_template::menu_begin =
"<div class=\"menu\">\n"
    "<form action=\"/search\" method=\"get\">\n"
        "<input class=search name=search type=text placeholder=\"Поиск корабликов...\" value=\"";

html_template menu_item_template::around =
{
        "\"><br>\n"
    "</form>\n"
    "\n"
    "<button>"
        "вооружение"
    "</button><br>\n"
    "<con-tent>\n"
        "<button>"
            "артиллерия"
        "</button><br>\n"
        "<con-tent>\n"
            "<a href=\"/armament/guns?sort=caliber,in_service&group=caliber&filter=class,0\">"
                "орудия"
            "</a><br>\n"
            "<a href=\"/armament/guns?sort=caliber,in_service&group=caliber&filter=class,1\">"
                "зенитные орудия"
            "</a><br>\n"
            "<a href=\"/armament/guns?sort=caliber,in_service&group=caliber&filter=class,2\">"
                "пулеметы"
            "</a><br>\n"
        "</con-tent>\n"
        "<button>"
            "торпедное вооружение"
        "</button><br>\n"
        "<con-tent>\n"
            "<a href=\"/armament/torpedo?group=caliber,in_service&sort=in_service\">"
                "торпеды"
            "</a><br>\n"
            "<a href=\"/armament/torpedo_tubes?group=caliber&sort=in_service\">"
                "торпедные аппараты"
            "</a><br>\n"
        "</con-tent>\n"
        "<button>"
            "авиация"
        "</button><br>\n"
        "<con-tent>\n",

            "<a href=\"/armament/catapult?group=class&sort=in_service\">"
                "катапульты"
            "</a><br>\n"
        "</con-tent>\n"
        "<button>"
            "средства обнаружения"
        "</button><br>\n"
        "<con-tent>\n"
            "<a href=\"/armament/searcher?group=power&sort=in_service&filter=class,8\">"
                "РЛС"
            "</a><br>\n"
            "<a href=\"/armament/searcher?group=power&sort=in_service&filter=class,13\">"
                "прожектора"
            "</a><br>\n"
        "</con-tent>\n"
        "<button>"
            "мины и глубинные бомбы"
        "</button><br>\n"
        "<con-tent>\n"
            "<a href=\"/armament/mines_charges?filter=class,4&sort=mass_ex\">"
                "глубинные бомбы"
            "</a><br>\n"
            "<a href=\"/armament/mines_charges?filter=class,6&sort=mass_ex\">"
                "мины"
            "</a><br>\n"
        "</con-tent>\n"
    "</con-tent>\n"
    "<div class=\"menu_link\">\n"
        "<a href=\"/documents\">"
            "документы"
        "</a><br>\n"
        "<a href=\"/?date=\">"
            "события"
        "</a><br>\n"
    "</div>\n"
    "<br>"
};

html_template_3 menu_item_template::link_template =
{
    "<a href=\"/aircraft?group=in_service&sort=type,in_service&filter=class,",
    "\">",
    "</a><br>\n"
};
    
html_template menu_item_template::menu_end =
{
    "<script>\n"
        "function collapse_menu (event)\n"
        "{\n"
            "event.target.classList.toggle(\"active\");\n"
            "let content = event.target;\n"
            "if (content.tagName != \"BUTTON\")\n"
                "content = content.parentElement;\n"
            "content = content.nextElementSibling.nextElementSibling;\n"
            "if (content.style.display === \"block\")\n"
                "content.style.display = \"none\";\n"
            "else\n"
                "content.style.display = \"block\";\n"
        "}\n"
        
        "var ships_to_compare = new Set();\n"
        "function toggle_to_compare (event)\n"
        "{\n"
            "if (event.target.checked)\n"
                "ships_to_compare.add(event.target);\n"
            "else\n"
                "ships_to_compare.delete(event.target);\n"
            "let link = \"/ship?\";\n"
            "let first = true;\n"
            "for (ship_to_compare of ships_to_compare)\n"
            "{\n"
                "if (!first)\n"
                    "link += \"&\";\n"
                "link += \"id=\" + ship_to_compare.getAttribute(\"id\").toString();\n"
                "first = false;\n"
            "}\n"
            "let compare_ships_button = document.getElementById(\"compare_ships_button\");\n"
            "compare_ships_button.setAttribute(\"href\", link);\n"
        "}\n"
        
        // set listeners for menu
        "for (menu_item of document.getElementsByClassName(\"menu\"))\n"
        "{\n"
            "for (coll_item of menu_item.getElementsByTagName(\"button\"))\n"
                "coll_item.addEventListener(\"click\", collapse_menu);\n"
            /*
            now generated by javascript
            "for (input of menu_item.getElementsByTagName(\"input\"))\n" 
                "if (input.getAttribute(\"type\") === \"checkbox\")\n"
                    "input.addEventListener(\"click\", toggle_to_compare);\n"
            */
            "for (link of menu_item.getElementsByTagName(\"a\"))\n" 
            "{\n"
                "if (link.parentElement.tagName == \"CON-TENT\")\n"
                "{\n"
                    "let prefix = \"/ship?id=\";\n"
                    "if (!link.getAttribute(\"href\").startsWith(prefix))"
                        "continue;\n"
                    "let ship_id = link.getAttribute(\"href\").substring(prefix.length);\n"
                    "link.insertAdjacentHTML(\"beforebegin\", \"<input type=checkbox id=\" + ship_id + \" onclick=toggle_to_compare(event)></input>\");\n"
                "}\n"
            "}\n"
        "}\n"
    "</script>\n"
    "<br>\n"
    "<div class=\"menu_link tooltip\">\n"
        "<a id=\"compare_ships_button\" href=\"/ship?\">сравнение корабликов</a>\n"
        "<span class=\"tooltiptext\">отмеченные галочками в меню</span>\n"
    "</div>\n",
    
"</div>\n"
};

