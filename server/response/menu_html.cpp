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
    "<style> \n"
        ".collapsible { \n"
            "background-color: #777777; \n"
            "color: white; \n"
            "cursor: crosshair; \n"
            "padding: 6px; \n"
            "border: none; \n"
            "text-align: left; \n"
            "outline: none; \n"
            "font-size: 15px; \n"
            "width: 100%; \n"
        "} \n"
        "\n"
        ".collapsible a { \n"
            "color: #ffffff; \n"
        "} \n"
        "\n"
        ".active { background-color: #555555; } \n"
        ".collapsible:hover { background-color: #ff5555; } \n"
        "\n"
        ".content { \n"
            "padding: 0px 0px 0px 20px; \n"
            "display: none; \n"
            "overflow: hidden; \n"
        "} \n"
        ".content > a { \n"
            "color: #000000; \n"
            "background-color: #ffffff; \n"
        "} \n"
        ".content > a:hover { color: #ff2222; } \n"
        "input { width: 97%; margin: 10px 20px 20px 0; }\n"
        "h2 { display: inline }\n"

        "tr.header    { background: #fff8dc; } \n"
        "tr.general   { background: #f8ffdc; } \n"
        "tr.guns      { background: #cdffcc; } \n"
        "tr.torpedo   { background: #ffdfdf; } \n"
        "tr.throwers  { background: #f8ffdc; } \n"
        "tr.searchers { background: #f8ffff; } \n"
        "tr.catapult  { background: #fff8dc; } \n"
        "tr.aircraft  { background: #dbffff; } \n"
        "th { min-width: 180px } \n"
        ".short_info th { min-width: 120px } \n"
        ".short_info a       { color: #222222 } \n"
        ".short_info a:hover { color: #ff2222 } \n"
    "</style>\n"
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

