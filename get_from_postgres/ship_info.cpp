#include "ship_requests.h"
#include "ship_info.h"
#include "template_request.h"


ship_requests::ship_info_t::list::list (pqxx::row const & value) :
    ship_id (value[0].as <int> ()),
    type_id (value[1].as <int> ()),
    class_id(value[2].as <int> ()),
    ship_ru (value[3].as <std::optional <std::string> > ()),
    ship_en (value[4].as <std::optional <std::string> > ()),
    type_ru (value[5].as <std::optional <std::string> > ()),
    type_en (value[6].as <std::optional <std::string> > ()),
    class_ru(value[7].as <std::optional <std::string> > ()),
    class_en(value[8].as <std::optional <std::string> > ()),
    commissioned(),
    sunk_date(),
    sunk_reason(value[11].as <std::optional <std::string> > ())
{
    std::optional <std::string> str_commissioned = value[ 9].as <std::optional <std::string> > ();
    std::optional <std::string> str_sunk_date    = value[10].as <std::optional <std::string> > ();
    commissioned = transform_optional(str_commissioned, get_date);
    sunk_date    = transform_optional(str_sunk_date,    get_date);
}

std::vector <ship_requests::ship_info_t::list> ship_requests::ship_info_t::get_list (std::string_view where)
{
    return request_to_db <list>
    (
        db,
        "select ship_list.id, ship_list.type_id, ship_list.class_id, \
                ship_list.name_ru, ship_list.name_en, \
                ship_types.name_ru, ship_types.name_en, \
                ship_class.name_ru, ship_class.name_en, \
                commissioned, sunk_date, sunk_reason \
         from ship_list \
         inner join ship_types on (ship_list.type_id = ship_types.id) \
         inner join ship_class on (ship_list.class_id = ship_class.id) ",
        where
    );
};


ship_requests::ship_info_t::general::general (pqxx::row const & value) :
    ship_id     (value[0].as <int> ()),
    displacement_standart(value[1].as <std::optional <double> > ()),
    displacement_full(value[2].as <std::optional <double> > ()),
    length      (value[3].as <std::optional <double> > ()),
    width       (value[4].as <std::optional <double> > ()),
    draft       (value[5].as <std::optional <double> > ()),
    crew        (value[6].as <std::optional <uint32_t> > ()),
    speed_max   (value[7].as <std::optional <double> > ()),
    speed_cruise(value[8].as <std::optional <double> > ()),
    range       (value[9].as <std::optional <double> > ()),
    date_from(),
    date_to()
{
    std::optional <std::string> str_commissioned = value[10].as <std::optional <std::string> > ();
    std::optional <std::string> str_sunk_date    = value[11].as <std::optional <std::string> > ();
    date_from = transform_optional(str_commissioned, get_date);
    date_to   = transform_optional(str_sunk_date,    get_date);
}
    
std::vector <ship_requests::ship_info_t::general> ship_requests::ship_info_t::get_general (std::string_view where)
{
    return request_to_db <general>
    (
        db,
        "select ship_id, displacement_standart, displacement_full, \
                length, width, draft, \
                crew, speed_max, speed_cruise, range, \
                date_from, date_to \
         from general ",
        where
    );
};

    
ship_requests::ship_info_t::types::types (pqxx::row const & value) :
    type_id (value[0].as <int> ()),
    type_ru (value[1].as <std::optional <std::string> > ()),
    type_en (value[2].as <std::optional <std::string> > ())
{}
    
std::vector <ship_requests::ship_info_t::types> ship_requests::ship_info_t::get_types (std::string_view where)
{
    return request_to_db <types>
    (
        db,
        "select id, name_ru, name_en from ship_types ",
        where
    );
};

    
ship_requests::ship_info_t::classes::classes (pqxx::row const & value) :
    class_id (value[0].as <int> ()),
    parent_id(value[1].as <std::optional <int> > ()),
    class_ru (value[2].as <std::optional <std::string> > ()),
    class_en (value[3].as <std::optional <std::string> > ())
{}
    
std::vector <ship_requests::ship_info_t::classes> ship_requests::ship_info_t::get_classes (std::string_view where)
{
    return request_to_db <classes>
    (
        db,
        "select id, parent_id, name_ru, name_en from ship_class ",
        where
    );
};


ship_requests::ship_info_t::sunk_dates::sunk_dates (pqxx::row const & value) :
    ship_id (value[0].as <int> ()),
    sunk_date()
{
    std::optional <std::string> str_sunk_date = value[1].as <std::optional <std::string> > ();
    sunk_date = transform_optional(str_sunk_date, get_date);
}
    
std::vector <ship_requests::ship_info_t::sunk_dates> ship_requests::ship_info_t::get_sunk_dates (std::string_view where)
{
    return request_to_db <sunk_dates>
    (
        db,
        "select id, sunk_date from ship_info ",
        where
    );
};


