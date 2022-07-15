#include "ship_requests.h"
#include "pictures.h"


ship_requests::pictures_t::picture::picture (pqxx::row const & value) :
    id          (value[0].as <int> ()),
    path_small  (value[1].as <std::string> ()),
    path_full   (value[2].as <std::string> ()),
    description (value[3].as <std::string> ())
{}

std::vector <ship_requests::pictures_t::picture> ship_requests::pictures_t::get_ship (std::string_view where)
{
    pqxx::result response = db->exec
    (
        std::string("select ship_id, path_small, path_full, description from pictures_ship ")
        +
        std::string(where)
    );
    std::vector <picture> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
};

std::vector <ship_requests::pictures_t::picture> ship_requests::pictures_t::get_aircraft (std::string_view where)
{
    pqxx::result response = db->exec
    (
        std::string("select aircraft_id, path_small, path_full, description from pictures_aircraft ")
        +
        std::string(where)
    );
    std::vector <picture> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
};

std::vector <ship_requests::pictures_t::picture> ship_requests::pictures_t::get_gun (std::string_view where)
{
    pqxx::result response = db->exec
    (
        std::string("select gun_id, path_small, path_full, description from pictures_gun ")
        +
        std::string(where)
    );
    std::vector <picture> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
};


