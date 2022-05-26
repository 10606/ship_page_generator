#ifndef RESPONSE_PARTIAL_H
#define RESPONSE_PARTIAL_H

#include <vector>

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

};

#endif

