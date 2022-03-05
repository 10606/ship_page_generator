#ifndef SHIP_EVENT_H
#define SHIP_EVENT_H

#include "ship_requests.h"


struct ship_requests::ship_event_t::classes
{
    classes (pqxx::row const & value);
    
    int class_id;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
};


struct ship_requests::ship_event_t::event
{
    event (pqxx::row const & value);
    
    int class_id;
    int ship_id;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
    std::optional <std::string> description;
};


#endif

