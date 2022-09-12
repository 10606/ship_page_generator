#ifndef BASE_COMPARATORS_H
#define BASE_COMPARATORS_H

#include <optional>
#include "base_compare_predict.h"

namespace comparators
{

template <typename T>
std::partial_ordering in_service (T const & a, T const & b)
{
    return compare_null_last(a.in_service, b.in_service);
}

template <typename T>
std::partial_ordering in_service_10th (T const & a, T const & b)
{
    return compare_date_10th(a.in_service, b.in_service);
}

template <typename T>
std::partial_ordering classes (T const & a, T const & b)
{
    return a.class_id <=> b.class_id;
}

template <typename T>
std::partial_ordering caliber (T const & a, T const & b)
{
    return a.caliber <=> b.caliber;
}

template <typename T>
std::partial_ordering name_ru (T const & a, T const & b)
{
    return compare_null_last(a.name_ru, b.name_ru);
}

template <typename T>
std::partial_ordering name_en (T const & a, T const & b)
{
    return compare_null_last(a.name_en, b.name_en);
}

template <typename T, typename member_t, member_t T::* member>
std::partial_ordering universal (T const & a, T const & b)
{
    return a.*member <=> b.*member;
}

};

#endif

