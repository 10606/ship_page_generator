#include <common.h>

#include "ship_info.h"


namespace
{

sunk_dates_t sunk_dates_impl ()
{
    std::vector <ship_requests::ship_info_t::sunk_dates> sunk_dates =
        ship_requests().ship_info.get_sunk_dates();
    
    sunk_dates_t answer;
    for (ship_requests::ship_info_t::sunk_dates const & item : sunk_dates)
        answer.insert({item.ship_id, {item.sunk_date}});
    return answer;
}

}


sunk_dates_t const & sunk_dates ()
{
    static const sunk_dates_t answer =
        sunk_dates_impl();
    return answer;
}

