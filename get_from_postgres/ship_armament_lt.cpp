#include "ship_requests.h"
#include "ship_armament_lt.h"


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
    pqxx::result response = db->exec
    (
        std::string
        (
            "select ship_id, mount_id, amount, \
                    date_from, date_to \
             from ship_guns "
        )
        +
        std::string(where)
    );
    std::vector <guns> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
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
    pqxx::result response = db->exec
    (
        std::string
        (
            "select ship_id, tube_id, amount, \
                    date_from, date_to \
             from ship_torpedo_tubes "
        )
        +
        std::string(where)
    );
    std::vector <torpedo_tubes> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
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
    pqxx::result response = db->exec
    (
        std::string
        (
            "select ship_id, throwers_id, amount, \
                    date_from, date_to \
             from ship_throwers "
        )
        +
        std::string(where)
    );
    std::vector <throwers> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
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
    pqxx::result response = db->exec
    (
        std::string
        (
            "select ship_id, searcher_id, amount, \
                    date_from, date_to \
             from ship_searchers "
        )
        +
        std::string(where)
    );
    std::vector <searchers> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
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
    pqxx::result response = db->exec
    (
        std::string
        (
            "select ship_id, catapult_id, amount, \
                    date_from, date_to \
             from ship_catapult "
        )
        +
        std::string(where)
    );
    std::vector <catapult> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
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
    pqxx::result response = db->exec
    (
        std::string
        (
            "select ship_id, aircraft_id, amount, amount_reserve, \
                    date_from, date_to \
             from ship_aircraft "
        )
        +
        std::string(where)
    );
    std::vector <aircraft> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
}
        

