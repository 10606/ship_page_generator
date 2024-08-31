#ifndef DOCUMENTS_H
#define DOCUMENTS_H

#include "documents_info.h"
#include "simple_string.h"
#include "parse_query.h"


struct document
{
    document (ship_requests * database);

    typedef ship_requests::documents_t::group group_t;
    typedef ship_requests::documents_t::document document_t;

    void response (simple_string & answer, std::string_view query, piece_t title);

    std::string_view menu_list = "<br>&nbsp<input type=\"checkbox\" onclick=toggle_preview(event) checked> превью: включено <br>\n";
    
private:
    std::string value;
};


#endif

