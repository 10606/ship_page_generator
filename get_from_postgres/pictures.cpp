#include "ship_requests.h"
#include "pictures.h"
#include "template_request.h"


ship_requests::pictures_t::picture::picture (pqxx::row const & value) :
    id          (value[0].as <int> ()),
    path_small  (value[1].as <std::string> ()),
    path_full   (value[2].as <std::string> ()),
    description (value[3].as <std::string> ())
{}

std::vector <ship_requests::pictures_t::picture> ship_requests::pictures_t::get_ship (std::string_view where)
{
    return request_to_db <picture> 
    (
        db, 
        "select ship_id, path_small, path_full, description from pictures_ship ", 
        where
    );
};

std::vector <ship_requests::pictures_t::picture> ship_requests::pictures_t::get_aircraft (std::string_view where)
{
    return request_to_db <picture> 
    (
        db, 
        "select aircraft_id, path_small, path_full, description from pictures_aircraft ", 
        where
    );
};

std::vector <ship_requests::pictures_t::picture> ship_requests::pictures_t::get_gun (std::string_view where)
{
    return request_to_db <picture> 
    (
        db, 
        "select gun_id, path_small, path_full, description from pictures_gun ", 
        where
    );
};

std::vector <ship_requests::pictures_t::picture> ship_requests::pictures_t::get_searcher (std::string_view where)
{
    return request_to_db <picture> 
    (
        db, 
        "select searcher_id, path_small, path_full, description from pictures_searcher ", 
        where
    );
};

std::vector <ship_requests::pictures_t::picture> ship_requests::pictures_t::get_catapult (std::string_view where)
{
    return request_to_db <picture> 
    (
        db, 
        "select catapult_id, path_small, path_full, description from pictures_catapult ", 
        where
    );
};


