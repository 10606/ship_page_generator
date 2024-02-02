#include "ship_requests.h"
#include "documents_info.h"
#include "template_request.h"


ship_requests::documents_t::group::group (pqxx::row const & value) :
    id          (value[0].as <int> ()),
    path_preview(value[1].as <std::optional <std::string> > ()),
    description (value[2].as <std::string> ())
{}

std::vector <ship_requests::documents_t::group> ship_requests::documents_t::get_groups (std::string_view where)
{
    return request_to_db <group> 
    (
        db, 
        "select id, path_preview, description from documents_group ", 
        where
    );
};


ship_requests::documents_t::document::document (pqxx::row const & value) :
    group_id        (value[0].as <int> ()),
    path_document   (value[1].as <std::string> ()),
    path_preview    (value[2].as <std::optional <std::string> > ()),
    description     (value[3].as <std::string> ())
{}

std::vector <ship_requests::documents_t::document> ship_requests::documents_t::get_documents (std::string_view where)
{
    return request_to_db <document> 
    (
        db, 
        "select group_id, path_document, path_preview, description from documents_general ", 
        where
    );
};

