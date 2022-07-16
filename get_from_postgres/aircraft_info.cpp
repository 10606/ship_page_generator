#include "ship_requests.h"
#include "aircraft_info.h"
#include "template_request.h"


ship_requests::aircraft_info_t::bombs::bombs (pqxx::row const & value) :
    bomb_id(value[0].as <int> ()),
    bomb_ru(value[1].as <std::optional <std::string> > ()),
    bomb_en(value[2].as <std::optional <std::string> > ()),
    
    mass   (value[3].as <std::optional <double> > ()),
    mass_ex(value[4].as <std::optional <double> > ()),
    in_service()
{
    std::optional <std::string> str_in_service = value[5].as <std::optional <std::string> > ();
    in_service = transform_optional(str_in_service, get_date);
}

std::vector <ship_requests::aircraft_info_t::bombs> ship_requests::aircraft_info_t::get_bombs (std::string_view where)
{
    return request_to_db <bombs>
    (
        db, 
        "select id, name_ru, name_en, mass, mass_ex, in_service from bombs ",
        where
    );
};


ship_requests::aircraft_info_t::classes::classes (pqxx::row const & value) :
    class_id (value[0].as <int> ()),
    parent_id(value[1].as <std::optional <int> > ()),
    class_ru (value[2].as <std::optional <std::string> > ()),
    class_en (value[3].as <std::optional <std::string> > ())
{}

std::vector <ship_requests::aircraft_info_t::classes> ship_requests::aircraft_info_t::get_classes (std::string_view where)
{
    return request_to_db <classes>
    (
        db,
        "select id, parent_id, name_ru, name_en from aircraft_class ",
        where
    );
};


ship_requests::aircraft_info_t::types::types (pqxx::row const & value) :
    type_id (value[0].as <int> ()),
    type_ru (value[1].as <std::optional <std::string> > ()),
    type_en (value[2].as <std::optional <std::string> > ())
{}

std::vector <ship_requests::aircraft_info_t::types> ship_requests::aircraft_info_t::get_types (std::string_view where)
{
    return request_to_db <types>
    (
        db,
        "select id, name_ru, name_en from aircraft_types ",
        where
    );
};


ship_requests::aircraft_info_t::list::list (pqxx::row const & value) :
    id          (value[0].as <int> ()),
    type_id     (value[1].as <int> ()),
    class_id    (value[2].as <int> ()),
    
    aircraft_ru (value[3].as <std::optional <std::string> > ()),
    aircraft_en (value[4].as <std::optional <std::string> > ()),
    type_ru     (value[5].as <std::optional <std::string> > ()),
    type_en     (value[6].as <std::optional <std::string> > ()),
    class_ru    (value[7].as <std::optional <std::string> > ()),
    class_en    (value[8].as <std::optional <std::string> > ()),

    crew        (value[ 9].as <std::optional <uint32_t> > ()),
    mass        (value[10].as <std::optional <double> > ()),
    max_mass    (value[11].as <std::optional <double> > ()),
    engine_power(value[12].as <std::optional <double> > ()),
    max_speed   (value[13].as <std::optional <double> > ()),
    cruise_speed(value[14].as <std::optional <double> > ()),
    range       (value[15].as <std::optional <double> > ()),
    range_with_tank(value[16].as <std::optional <double> > ()),
    ceiling     (value[17].as <std::optional <double> > ()),
    time_to_altitude(value[18].as <std::optional <double> > ()),
    
    in_service()
{
    std::optional <std::string> str_in_service = value[19].as <std::optional <std::string> > ();
    in_service = transform_optional(str_in_service, get_date);
}

std::vector <ship_requests::aircraft_info_t::list> ship_requests::aircraft_info_t::get_list (std::string_view where)
{
    return request_to_db <list>
    (
        db,
        "select aircraft_list.id, aircraft_types.id, aircraft_class.id, \
                aircraft_list.name_ru,  aircraft_list.name_en, \
                aircraft_types.name_ru, aircraft_types.name_en, \
                aircraft_class.name_ru, aircraft_class.name_en, \
                crew, mass, max_mass, engine_power, \
                max_speed, cruise_speed, range, range_with_tank, \
                ceiling, time_to_altitude, in_service \
         from aircraft_list \
         inner join aircraft_types on (aircraft_list.type_id = aircraft_types.id) \
         inner join aircraft_class on (aircraft_list.class_id = aircraft_class.id) ",
        where
    );
};
    
    
