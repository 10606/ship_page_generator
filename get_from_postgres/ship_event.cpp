#include "ship_requests.h"
#include "ship_event.h"
#include "template_request.h"


ship_requests::ship_event_t::classes::classes (pqxx::row const & value) :
    class_id (value[0].as <int> ()),
    class_ru (value[1].as <std::optional <std::string> > ()),
    class_en (value[2].as <std::optional <std::string> > ())
{}

std::vector <ship_requests::ship_event_t::classes> ship_requests::ship_event_t::get_classes (std::string_view where)
{
    return request_to_db <classes>
    (
        db,
        "select id, name_ru, name_en from event_class ",
        where
    );
};


ship_requests::ship_event_t::event::event (pqxx::row const & value) :
    class_id (value[0].as <int> ()),
    ship_id  (value[1].as <int> ()),
    class_ru (value[2].as <std::optional <std::string> > ()),
    class_en (value[3].as <std::optional <std::string> > ()),
    date_from(),
    date_to  (),
    description(value[6].as <std::optional <std::string> > ())
{
    std::optional <std::string> str_date_from = value[4].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    std::optional <std::string> str_date_to   = value[5].as <std::optional <std::string> > ();
    date_to   = transform_optional(str_date_to, get_date);
}

std::vector <ship_requests::ship_event_t::event> ship_requests::ship_event_t::get_event (std::string_view where)
{
    return request_to_db <event>
    (
        db,
        "select class_id, ship_id, name_ru, \
                name_en, date_from, date_to, description \
         from ship_event_list \
         inner join event_class on (ship_event_list.class_id = event_class.id) ",
        where
    );
};


ship_requests::ship_event_t::event_lt::event_lt (pqxx::row const & value) :
    ship_id  (value[0].as <int> ()),
    date_from(),
    date_to  ()
{
    std::optional <std::string> str_date_from = value[1].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    std::optional <std::string> str_date_to   = value[2].as <std::optional <std::string> > ();
    date_to   = transform_optional(str_date_to, get_date);
}

std::vector <ship_requests::ship_event_t::event_lt> ship_requests::ship_event_t::get_event_lt (std::string_view where)
{
    return request_to_db <event_lt>
    (
        db,
        "select ship_id,  date_from, date_to from ship_event_list ",
        where
    );
};


ship_requests::ship_event_t::event_lt_descr::event_lt_descr (pqxx::row const & value) :
    ship_id  (value[0].as <int> ()),
    class_id (value[1].as <int> ()),
    date_from(),
    date_to  (),
    class_ru (value[4].as <std::optional <std::string> > ()),
    description(value[5].as <std::optional <std::string> > ())
{
    std::optional <std::string> str_date_from = value[2].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    std::optional <std::string> str_date_to   = value[3].as <std::optional <std::string> > ();
    date_to   = transform_optional(str_date_to, get_date);
}

std::vector <ship_requests::ship_event_t::event_lt_descr> ship_requests::ship_event_t::get_event_lt_descr (std::string_view where)
{
    return request_to_db <event_lt_descr>
    (
        db,
        "select ship_id, class_id, \
                date_from, date_to, \
                name_ru, description \
         from ship_event_list \
         inner join event_class on (ship_event_list.class_id = event_class.id) ",
        where
    );
};


size_t ship_requests::ship_event_t::count (std::string_view where)
{
    pqxx::result response = db->exec
    (
        std::string("select count(*) from ship_event_list ") + std::string(where)
    );
    if (response.empty())
        return 0;
    
    return (response[0][0].as <size_t> ());
};


