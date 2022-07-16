#include "ship_requests.h"
#include "pictures.h"


ship_requests::pictures_t::picture::picture (pqxx::row const & value) :
    id          (value[0].as <int> ()),
    path_small  (value[1].as <std::string> ()),
    path_full   (value[2].as <std::string> ()),
    description (value[3].as <std::string> ())
{}

struct get_picture
{
    get_picture (ship_database * _db, std::string_view _query) :
        db(_db),
        query(_query)
    {}

    typedef ship_requests::pictures_t::picture picture;

    std::vector <picture> operator () (std::string_view where)
    {
        pqxx::result response = db->exec(query + std::string(where));
        std::vector <picture> answer;
        
        for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
            answer.emplace_back(*row);  
        return answer;
    };

private:
    ship_database * db;
    std::string query;
};

std::vector <ship_requests::pictures_t::picture> ship_requests::pictures_t::get_ship (std::string_view where)
{
    static get_picture get(db, "select ship_id, path_small, path_full, description from pictures_ship ");
    return get(where);
};

std::vector <ship_requests::pictures_t::picture> ship_requests::pictures_t::get_aircraft (std::string_view where)
{
    static get_picture get(db, "select aircraft_id, path_small, path_full, description from pictures_aircraft ");
    return get(where);
};

std::vector <ship_requests::pictures_t::picture> ship_requests::pictures_t::get_gun (std::string_view where)
{
    static get_picture get(db, "select gun_id, path_small, path_full, description from pictures_gun ");
    return get(where);
};

std::vector <ship_requests::pictures_t::picture> ship_requests::pictures_t::get_searcher (std::string_view where)
{
    static get_picture get(db, "select searcher_id, path_small, path_full, description from pictures_searcher ");
    return get(where);
};


