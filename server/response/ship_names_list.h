#ifndef SHIP_NAMES_LIST_H
#define SHIP_NAMES_LIST_H

#include "ship_requests.h"
#include "ship_info.h"
#include "simple_string.h"
#include "armament_links.h"

struct ship_names_list
{
    ship_names_list (ship_requests * database)
    {
        std::vector <ship_info_long> ship_names =
            database->ship_info.get_list("order by ((select get_ancestor_by_id (ship_list.class_id)), ship_list.class_id, "
                                                    "ship_list.type_id, commissioned, ship_list.name_ru,  ship_list.id)");

        _names.reserve(ship_names.size());
        for (ship_info_long const & ship_info : ship_names)
            _names.emplace_back(ship_info);
    }

    typedef ship_requests::ship_info_t::list ship_info_long;
    
    struct ship_info_t
    {
        ship_info_t (ship_info_long value) :
            class_id(value.class_id),
            name_en(value.ship_en.value_or("")),
            name_ru(value.ship_ru.value_or("")),
            commissioned(value.commissioned),
            sunk_date   (value.sunk_date),
            answer()
        {
            answer.append("<tr><td><b>")
                  .append(armament_links::base("/ship?id=" + std::to_string(value.ship_id), value.ship_ru.value_or("--")))
                  .append(" ")
                  .append("</b></td><th>");
            if (value.commissioned)
                answer.append(to_string(*value.commissioned));
            answer.append("</th><td>-</td><th>");
            if (value.sunk_date)
                answer.append(to_string(*value.sunk_date));
            answer.append("</th><td>");
            if (value.class_ru || value.type_ru)
            {
                answer.append("(")
                      .append(value.class_ru.value_or(""));
                if (value.type_ru)
                    answer.append(" <a href=\"/ship?type_id=")
                          .append(std::to_string(value.type_id))
                          .append("\">")
                          .append("типа ")
                          .append(*value.type_ru)
                          .append("<a>");
                answer.append(")");
            }
            answer.append("</td></tr>");
        }

        int class_id;
        std::string name_en;
        std::string name_ru;
        std::optional <std::chrono::year_month_day> commissioned;
        std::optional <std::chrono::year_month_day> sunk_date;
        std::string answer;
    };
    
    struct add_ship_t
    {
        add_ship_t (std::vector <ship_info_t> const & _names, simple_string & _answer) :
            names(_names),
            answer(_answer),
            class_id(),
            old_pos()
        {}

        void operator () (uint32_t pos)
        {
            if (old_pos == pos)
                return;
            old_pos = pos;
            if (names[pos].class_id != class_id) [[unlikely]]
            {
                if (class_id) [[likely]]
                    answer.append("</table>");
                answer.append("<br><table>");
                class_id = names[pos].class_id;
            }
            answer.append(names[pos].answer);
        }

        void close ()
        {
            if (class_id)
                answer.append("</table>");
            class_id.reset(); // idempotent
        }
        
        ~add_ship_t ()
        {
            try
            {
                close();
            }
            catch (...)
            {}
        }
        
    private:
        std::vector <ship_info_t> const & names;
        simple_string & answer;
        std::optional <int> class_id;
        std::optional <uint32_t> old_pos; // don't add twice
    };

    std::vector <ship_info_t> const & names () const noexcept
    {
        return _names;
    }
    
private:
    std::vector <ship_info_t> _names;
};

#endif

