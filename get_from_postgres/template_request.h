#ifndef TEMPLATE_REQUEST_H
#define TEMPLATE_REQUEST_H

#include "ship_requests.h"

template <typename T>
std::vector <T>
request_to_db (ship_database * db, std::string_view query, std::string_view where)
{
    pqxx::result response = where.empty()? 
                            db->exec(query) :
                            db->exec(std::string(query).append(where));
    std::vector <T> answer;
    
    for (pqxx::result::const_iterator row = response.begin(); row != response.end(); ++row)
        answer.emplace_back(*row);  
    return answer;
}

#endif

