#ifndef DOCUMENTS_INFO_H
#define DOCUMENTS_INFO_H

#include "ship_requests.h"


struct ship_requests::documents_t::group
{
    group (pqxx::row const & value);

    int id;
    std::optional <std::string> path_preview;
    std::string description;
};


struct ship_requests::documents_t::document
{
    document (pqxx::row const & value);

    int group_id;
    std::string path_document;
    std::optional <std::string> path_preview;
    std::string description;
};


#endif

