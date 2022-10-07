#ifndef SHIP_INFO_H
#define SHIP_INFO_H

#include "ship_requests.h"


struct ship_requests::ship_info_t::list
{
    list (pqxx::row const & value);

    int ship_id;
    int type_id;
    int class_id;
    std::optional <std::string> ship_ru;
    std::optional <std::string> ship_en;
    std::optional <std::string> type_ru;
    std::optional <std::string> type_en;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    
    std::optional <std::chrono::year_month_day> commissioned;
    std::optional <std::chrono::year_month_day> sunk_date;
    std::optional <std::string> sunk_reason;
};


struct ship_requests::ship_info_t::general
{
    general (pqxx::row const & value);

    int ship_id;
    std::optional <double> displacement_standart;
    std::optional <double> displacement_full;
    std::optional <double> length;
    std::optional <double> width;
    std::optional <double> draft;
    std::optional <uint32_t> crew;
    std::optional <double> speed_max;
    std::optional <double> speed_cruise;
    std::optional <double> range;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};


struct ship_requests::ship_info_t::types
{
    types (pqxx::row const & value);

    int type_id;
    std::optional <std::string> type_ru;
    std::optional <std::string> type_en;
};


struct ship_requests::ship_info_t::classes
{
    classes (pqxx::row const & value);

    int class_id;
    std::optional <int> parent_id;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
};


struct ship_requests::ship_info_t::sunk_dates
{
    sunk_dates (pqxx::row const & value);

    int ship_id;
    std::optional <std::chrono::year_month_day> sunk_date;
};


#endif

