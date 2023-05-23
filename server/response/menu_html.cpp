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
"<div class = \"menu\"> \n"
    "<form action = \"/search\" method = \"get\">\n"
        "<input name = \"search\" type = \"text\" placeholder = \"Поиск корабликов...\" value = \"";

html_template menu_item_template::around =
{
        "\"><br>\n"
    "</form>\n"
    "\n"
    "\n"
    "<button type = \"button\" class = \"collapsible\">\n"
        "вооружение\n"
    "</button><br>\n"
    "<div class = \"content\">\n"
        "<button type = \"button\" class = \"collapsible\">\n"
            "артиллерия\n"
        "</button><br>\n"
        "<div class = \"content\">\n"
            "<a href=\"/armament/guns?sort=caliber,in_service&group=caliber&filter=class,0\">\n"
                "орудия\n"
            "</a><br>\n"
            "<a href=\"/armament/guns?sort=caliber,in_service&group=caliber&filter=class,1\">\n"
                "зенитные орудия\n"
            "</a><br>\n"
            "<a href=\"/armament/guns?sort=caliber,in_service&group=caliber&filter=class,2\">\n"
                "пулеметы\n"
            "</a><br>\n"
        "</div>\n"
        "\n"
        "<button type = \"button\" class = \"collapsible\">\n"
            "торпедное вооружение\n"
        "</button><br>\n"
        "<div class = \"content\">\n"
            "<a href=\"/armament/torpedo?group=caliber,in_service&sort=in_service\">\n"
                "торпеды\n"
            "</a><br>\n"
            "<a href=\"/armament/torpedo_tubes?group=caliber&sort=in_service\">\n"
                "торпедные аппараты\n"
            "</a><br>\n"
        "</div>\n"
        "\n"
        "<button type = \"button\" class = \"collapsible\">\n"
            "авиация\n"
        "</button><br>\n"
        "<div class = \"content\">\n",

            "<a href=\"/armament/catapult?group=class&sort=in_service\">\n"
                "катапульты\n"
            "</a><br>\n"
        "</div>\n"
        "\n"
        "<button type = \"button\" class = \"collapsible\">\n"
            "средства обнаружения\n"
        "</button><br>\n"
        "<div class = \"content\">\n"
            "<a href=\"/armament/searcher?group=power&sort=in_service&filter=class,8\">\n"
                "РЛС\n"
            "</a><br>\n"
            "<a href=\"/armament/searcher?group=power&sort=in_service&filter=class,13\">\n"
                "прожектора\n"
            "</a><br>\n"
        "</div>\n"
        "\n"
        "<button type = \"button\" class = \"collapsible\">\n"
            "мины и глубинные бомбы\n"
        "</button><br>\n"
        "<div class = \"content\">\n"
            "<a href=\"/armament/mines_charges?filter=class,4&sort=mass_ex\">\n"
                "глубинные бомбы\n"
            "</a><br>\n"
            "<a href=\"/armament/mines_charges?filter=class,6&sort=mass_ex\">\n"
                "мины\n"
            "</a><br>\n"
        "</div>\n"
    "</div>\n"
    "<br>"
};

html_template_3 menu_item_template::link_template =
{
    "<a href=\"/aircraft?group=in_service&sort=type,in_service&filter=class,",
    "\">\n",
    "\n</a><br>\n"
};
    
std::string_view menu_item_template::menu_end =
    "<script>\n"
        "var coll = document.getElementsByClassName(\"collapsible\");\n"
        "var i;\n"
        "\n"
        "for (i = 0; i < coll.length; i++) {\n"
            "coll[i].addEventListener(\"click\", function() {\n"
                "this.classList.toggle(\"active\");\n"
                "var content = this.nextElementSibling.nextElementSibling;\n"
                "if (content.style.display === \"block\") {\n"
                    "content.style.display = \"none\";\n"
                "} else {\n"
                    "content.style.display = \"block\";\n"
                "}\n"
            "});\n"
        "}\n"
    "</script>\n"
"</div>\n";

menu_item_template::menu_item_template (ship_requests * database) :
    something_needed(generate_aircraft_links(database, around, link_template)),
    all{menu_begin, something_needed, menu_end}
{}

