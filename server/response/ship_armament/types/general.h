#ifndef GENERAL_H
#define GENERAL_H

#include <vector>
#include <unordered_map>
#include <chrono>
#include <cmath>
#include "allocator.h"
#include "ship_requests.h"


struct ship_general
{
    ship_general (ship_requests & database, std::string_view _new_line);

    struct responses_common_t
    {
        static const constexpr bool group = 0;
        uint8_t compare;
        std::string_view group_name;
    };
    
    struct p_response_t : responses_common_t
    {
        std::string data;
    };

    struct response_t : responses_common_t
    {
        response_t () = default;
    
        response_t (p_response_t const & value) :
            responses_common_t(value),
            data_begin(),
            data_end(value.data)
        {}
    
        std::string_view data_begin;
        std::string_view data_end;
    };

    std::vector <response_t, allocator_for_temp <response_t> >
    response (int id, std::chrono::year_month_day date) const;

private:
    struct response_with_time_label
    {
        std::vector <p_response_t> answer;
        std::optional <std::chrono::year_month_day> date_from;
        std::optional <std::chrono::year_month_day> date_to;
    };
    
    typedef ship_requests::ship_info_t::general general_t;
    std::unordered_map <int, std::vector <response_with_time_label> > ship_general_list;
    
    response_with_time_label partial_response (general_t const & general);
    
    std::string new_line;
};


#endif

