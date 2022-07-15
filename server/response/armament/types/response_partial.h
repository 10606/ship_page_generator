#ifndef RESPONSE_PARTIAL_H
#define RESPONSE_PARTIAL_H

#include <vector>
#include <stddef.h>
#include "pictures.h"

namespace partial
{

template <typename type_t, typename type_partial>
std::vector <type_partial> partial_response (std::vector <type_t> const & list)
{
    std::vector <type_partial> answer;
    answer.reserve(list.size());
    for (size_t i = 0; i != list.size(); ++i)
        answer.emplace_back(list[i], i);
    return answer;
}

template <typename type_t, typename type_text>
std::vector <type_text> text_response (std::vector <type_t> const & list)
{
    std::vector <type_text> answer;
    answer.reserve(list.size());
    for (size_t i = 0; i != list.size(); ++i)
        answer.emplace_back(list[i]);
    return answer;
}

template <typename type_t>
requires requires { std::is_same_v <int, std::decay_t <decltype(type_t::id)>>; }
std::vector <std::vector <ship_requests::pictures_t::picture> >
pictures_response
(std::vector <ship_requests::pictures_t::picture> const & all, std::vector <type_t> const & list)
{
    typedef ship_requests::pictures_t::picture picture_t;
    std::unordered_map <int, std::vector <picture_t> > grouped;

    for (picture_t const & picture : all)
        grouped[picture.id].push_back(picture);

    std::vector <std::vector <picture_t> > answer;
    answer.resize(list.size());
    for (size_t i = 0; i != list.size(); ++i)
        answer[i] = grouped[list[i].id];
    return answer;
}

};

#endif

