#ifndef TEMPLATE_REQUEST_H
#define TEMPLATE_REQUEST_H

#include "ship_requests.h"

template <typename T>
std::vector <T>
request_to_db (ship_database * db, std::string_view query)
{
    pqxx::result response = db->exec(query);
    std::vector <T> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
}

#endif

