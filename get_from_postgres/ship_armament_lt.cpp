#include "ship_requests.h"
#include "ship_armament_lt.h"
#include "template_request.h"


ship_requests::ship_armament_lt_t::guns::guns (pqxx::row const & value) :
        ship_id    (value[0].as <int> ()),
        mount_id   (value[1].as <int> ()),
        mount_count(value[2].as <uint32_t> ()),
        date_from(),
        date_to()
{
    std::optional <std::string> str_date_from = value[3].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[4].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}

std::vector <ship_requests::ship_armament_lt_t::guns> ship_requests::ship_armament_lt_t::get_guns (std::string_view where)
{
    return request_to_db <guns>
    (
        db,
        std::string
        (
            "select ship_id, mount_id, amount, \
                    date_from, date_to \
             from ship_guns "
        )
        .append(where)
    );
}
        
        
ship_requests::ship_armament_lt_t::torpedo_tubes::torpedo_tubes (pqxx::row const & value) :
        ship_id     (value[0].as <int> ()),
        tube_id     (value[1].as <int> ()),
        mount_count (value[2].as <uint32_t> ()),
        date_from(),
        date_to()
{
    std::optional <std::string> str_date_from = value[3].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[4].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}

std::vector <ship_requests::ship_armament_lt_t::torpedo_tubes> ship_requests::ship_armament_lt_t::get_torpedo_tubes (std::string_view where)
{
    return request_to_db <torpedo_tubes>
    (
        db,
        std::string
        (
            "select ship_id, tube_id, amount, \
                    date_from, date_to \
             from ship_torpedo_tubes "
        )
        .append(where)
    );
}
        
        
ship_requests::ship_armament_lt_t::throwers::throwers (pqxx::row const & value) :
        ship_id     (value[0].as <int> ()),
        throwers_id (value[1].as <int> ()),
        mount_count (value[2].as <uint32_t> ()),
        date_from(),
        date_to()
{
    std::optional <std::string> str_date_from = value[3].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[4].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}

std::vector <ship_requests::ship_armament_lt_t::throwers> ship_requests::ship_armament_lt_t::get_throwers (std::string_view where)
{
    return request_to_db <throwers>
    (
        db,
        std::string
        (
            "select ship_id, throwers_id, amount, \
                    date_from, date_to \
             from ship_throwers "
        )
        .append(where)
    );
}
        
        
ship_requests::ship_armament_lt_t::searchers::searchers (pqxx::row const & value) :
        ship_id     (value[0].as <int> ()),
        searcher_id (value[1].as <int> ()),
        count       (value[2].as <uint32_t> ()),
        date_from(),
        date_to()
{
    std::optional <std::string> str_date_from = value[3].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[4].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}

std::vector <ship_requests::ship_armament_lt_t::searchers> ship_requests::ship_armament_lt_t::get_searchers (std::string_view where)
{
    return request_to_db <searchers>
    (
        db,
        std::string
        (
            "select ship_id, searcher_id, amount, \
                    date_from, date_to \
             from ship_searchers "
        )
        .append(where)
    );
}
        
        
ship_requests::ship_armament_lt_t::catapult::catapult (pqxx::row const & value) :
        ship_id     (value[0].as <int> ()),
        catapult_id (value[1].as <int> ()),
        count       (value[2].as <uint32_t> ()),
        date_from(),
        date_to()
{
    std::optional <std::string> str_date_from = value[3].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[4].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}
        
std::vector <ship_requests::ship_armament_lt_t::catapult> ship_requests::ship_armament_lt_t::get_catapult (std::string_view where)
{
    return request_to_db <catapult>
    (
        db,
        std::string
        (
            "select ship_id, catapult_id, amount, \
                    date_from, date_to \
             from ship_catapult "
        )
        .append(where)
    );
}
        
        
ship_requests::ship_armament_lt_t::aircraft::aircraft (pqxx::row const & value) :
        ship_id     (value[0].as <int> ()),
        aircraft_id (value[1].as <int> ()),
        count       (value[2].as <uint32_t> ()),
        count_reserve(value[3].as <uint32_t> ()),
        date_from(),
        date_to()
{
    std::optional <std::string> str_date_from = value[4].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[5].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}
        
std::vector <ship_requests::ship_armament_lt_t::aircraft> ship_requests::ship_armament_lt_t::get_aircraft (std::string_view where)
{
    return request_to_db <aircraft>
    (
        db,
        std::string
        (
            "select ship_id, aircraft_id, amount, amount_reserve, \
                    date_from, date_to \
             from ship_aircraft "
        )
        .append(where)
    );
}
        

