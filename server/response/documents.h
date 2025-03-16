#ifndef DOCUMENTS_H
#define DOCUMENTS_H

#include "response.h"
#include "documents_info.h"
#include "simple_string.h"
#include "parse_query.h"


struct document : response_base
{
    document (ship_requests & database);

    typedef ship_requests::documents_t::group group_t;
    typedef ship_requests::documents_t::document document_t;

    virtual void response (simple_string & answer, std::string_view query, piece_t title) override;

    virtual std::string_view additional_in_menu () const override
    {
        return "<br>&nbsp<input type=checkbox onchange=toggle_preview(event) checked> превью: включено <br>\n";
    }
    
private:
    std::string value;
};


#endif

