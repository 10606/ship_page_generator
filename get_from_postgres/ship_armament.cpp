#include "ship_requests.h"
#include "ship_armament.h"


ship_requests::ship_armament_t::guns::guns (pqxx::row const & value) :
        class_id(value[0].as <int> ()),
        class_ru(value[1].as <std::optional <std::string> > ()),
        class_en(value[2].as <std::optional <std::string> > ()),
        gun_id  (value[3].as <int> ()),
        caliber (value[4].as <std::optional <double> > ()),
        length  (value[5].as <std::optional <double> > ()),
        gun_ru  (value[6].as <std::optional <std::string> > ()),
        gun_en  (value[7].as <std::optional <std::string> > ()),
        mount_ru(value[8].as <std::optional <std::string> > ()),
        mount_en(value[9].as <std::optional <std::string> > ()),
        angle   (value[10].as <std::optional <double> > ()),
        gun_count(value[11].as <uint32_t> ()),
        mount_count(value[12].as <uint32_t> ()),
        date_from(),
        date_to()
{
    std::optional <std::string> str_date_from = value[13].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[14].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}

std::vector <ship_requests::ship_armament_t::guns> ship_requests::ship_armament_t::get_guns (std::string_view where)
{
    pqxx::result response = db->exec
    (
        std::string
        (
            "select gun_class.id, gun_class.name_ru, gun_class.name_en, \
                    gun_list.id, caliber, length, gun_list.name_ru, gun_list.name_en, \
                    gun_mount.name_ru, gun_mount.name_en, angle, gun_mount.gun_count, \
                    amount, date_from, date_to \
             from ship_guns \
             inner join gun_mount on (ship_guns.mount_id = gun_mount.id) \
             inner join gun_list on (gun_mount.gun_id = gun_list.id) \
             inner join gun_class on (gun_list.class_id = gun_class.id) "
        )
        +
        std::string(where)
    );
    std::vector <guns> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
}
        
        
ship_requests::ship_armament_t::torpedo_tubes::torpedo_tubes (pqxx::row const & value) :
        class_id    (value[0].as <int> ()),
        class_ru    (value[1].as <std::optional <std::string> > ()),
        class_en    (value[2].as <std::optional <std::string> > ()),
        tube_id     (value[3].as <int> ()),
        caliber     (value[4].as <std::optional <double> > ()),
        tubes_count (value[5].as <std::optional <uint32_t> > ()),
        tube_ru     (value[6].as <std::optional <std::string> > ()),
        tube_en     (value[7].as <std::optional <std::string> > ()),
        mount_count (value[8].as <uint32_t> ()),
        date_from(),
        date_to()
{
    std::optional <std::string> str_date_from = value[ 9].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[10].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}

std::vector <ship_requests::ship_armament_t::torpedo_tubes> ship_requests::ship_armament_t::get_torpedo_tubes (std::string_view where)
{
    pqxx::result response = db->exec
    (
        std::string
        (
            "select gun_class.id, gun_class.name_ru, gun_class.name_en, \
                    torpedo_tubes.id, caliber, tubes_count, \
                    torpedo_tubes.name_ru, torpedo_tubes.name_en, \
                    amount, date_from, date_to \
             from ship_torpedo_tubes \
             inner join torpedo_tubes on (ship_torpedo_tubes.tube_id = torpedo_tubes.id) \
             inner join gun_class on (torpedo_tubes.class_id = gun_class.id) "
        )
        +
        std::string(where)
    );
    std::vector <torpedo_tubes> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
}
        
        
ship_requests::ship_armament_t::throwers::throwers (pqxx::row const & value) :
        class_id    (value[0].as <int> ()),
        class_ru    (value[1].as <std::optional <std::string> > ()),
        class_en    (value[2].as <std::optional <std::string> > ()),
        throwers_id (value[3].as <int> ()),
        caliber     (value[4].as <std::optional <double> > ()),
        tubes_count (value[5].as <std::optional <uint32_t> > ()),
        thrower_ru  (value[6].as <std::optional <std::string> > ()),
        thrower_en  (value[7].as <std::optional <std::string> > ()),
        mount_count (value[8].as <uint32_t> ()),
        date_from(),
        date_to()
{
    std::optional <std::string> str_date_from = value[ 9].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[10].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}

std::vector <ship_requests::ship_armament_t::throwers> ship_requests::ship_armament_t::get_throwers (std::string_view where)
{
    pqxx::result response = db->exec
    (
        std::string
        (
            "select gun_class.id, gun_class.name_ru, gun_class.name_en, \
                    throwers.id, caliber, tubes_count, \
                    throwers.name_ru, throwers.name_en, \
                    amount, date_from, date_to \
             from ship_throwers \
             inner join throwers on (ship_throwers.throwers_id = throwers.id) \
             inner join gun_class on (throwers.class_id = gun_class.id) "
        )
        +
        std::string(where)
    );
    std::vector <throwers> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
}
        
        
ship_requests::ship_armament_t::searchers::searchers (pqxx::row const & value) :
        class_id    (value[0].as <int> ()),
        class_ru    (value[1].as <std::optional <std::string> > ()),
        class_en    (value[2].as <std::optional <std::string> > ()),
        searcher_id (value[3].as <int> ()),
        searcher_ru (value[4].as <std::optional <std::string> > ()),
        searcher_en (value[5].as <std::optional <std::string> > ()),
        count       (value[6].as <uint32_t> ()),
        date_from(),
        date_to()
{
    std::optional <std::string> str_date_from = value[7].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[8].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}

std::vector <ship_requests::ship_armament_t::searchers> ship_requests::ship_armament_t::get_searchers (std::string_view where)
{
    pqxx::result response = db->exec
    (
        std::string
        (
            "select gun_class.id, gun_class.name_ru, gun_class.name_en, \
                    searchers.id, searchers.name_ru, searchers.name_en, \
                    amount, date_from, date_to \
             from ship_searchers \
             inner join searchers on (ship_searchers.searcher_id = searchers.id) \
             inner join gun_class on (searchers.class_id = gun_class.id) "
        )
        +
        std::string(where)
    );
    std::vector <searchers> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
}
        
        
ship_requests::ship_armament_t::catapult::catapult (pqxx::row const & value) :
        class_id    (value[0].as <int> ()),
        class_ru    (value[1].as <std::optional <std::string> > ()),
        class_en    (value[2].as <std::optional <std::string> > ()),
        catapult_id (value[3].as <int> ()),
        catapult_ru (value[4].as <std::optional <std::string> > ()),
        catapult_en (value[5].as <std::optional <std::string> > ()),
        count       (value[6].as <uint32_t> ()),
        date_from(),
        date_to()
{
    std::optional <std::string> str_date_from = value[7].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[8].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}
        
std::vector <ship_requests::ship_armament_t::catapult> ship_requests::ship_armament_t::get_catapult (std::string_view where)
{
    pqxx::result response = db->exec
    (
        std::string
        (
            "select catapult_class.id, catapult_class.name_ru, catapult_class.name_en, \
                    catapult.id, catapult.name_ru, catapult.name_en, \
                    amount, date_from, date_to \
             from ship_catapult \
             inner join catapult on (ship_catapult.catapult_id = catapult.id) \
             inner join catapult_class on (catapult.class_id = catapult_class.id) "
        )
        +
        std::string(where)
    );
    std::vector <catapult> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
}
        
        
ship_requests::ship_armament_t::aircraft::aircraft (pqxx::row const & value) :
        class_id    (value[0].as <int> ()),
        class_ru    (value[1].as <std::optional <std::string> > ()),
        class_en    (value[2].as <std::optional <std::string> > ()),
        aircraft_id (value[3].as <int> ()),
        aircraft_ru (value[4].as <std::optional <std::string> > ()),
        aircraft_en (value[5].as <std::optional <std::string> > ()),
        count       (value[6].as <uint32_t> ()),
        date_from(),
        date_to()
{
    std::optional <std::string> str_date_from = value[7].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[8].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}
        
std::vector <ship_requests::ship_armament_t::aircraft> ship_requests::ship_armament_t::get_aircraft (std::string_view where)
{
    pqxx::result response = db->exec
    (
        std::string
        (
            "select aircraft_class.id, aircraft_class.name_ru, aircraft_class.name_en, \
                    aircraft_list.id, aircraft_list.name_ru, aircraft_list.name_en, \
                    amount, date_from, date_to \
             from ship_aircraft \
             inner join aircraft_list on (ship_aircraft.aircraft_id = aircraft_list.id) \
             inner join aircraft_class on (aircraft_list.class_id = aircraft_class.id) "
        )
        +
        std::string(where)
    );
    std::vector <aircraft> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
}
        

