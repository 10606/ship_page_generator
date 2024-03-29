#ifndef SHIP_PROPULSION_H
#define SHIP_PROPULSION_H

#include "ship_requests.h"

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <array>
#include <string_view>
#include <bitset>
#include <optional>
#include <memory>
#include <vector>

#include "date_to_str.h"
#include "template_request.h"


template <typename T>
concept pointer = 
    std::is_pointer_v <T> ||
    requires (T a)
    {
        typename T::pointer;
        typename T::element_type;
        {a.operator -> ()} -> std::same_as <typename T::pointer>;
        {a.operator *  ()} -> std::same_as <typename T::element_type &>;
    };

struct ship_requests::propulsion_t::all_items
{
    all_items (pqxx::row const & value) :
        object_id(value[0].as <int> ()),
        item_id  (value[1].as <int> ()),
        count    (value[2].as <std::optional <uint32_t> > ())
    {}
    
    int object_id;
    int item_id;
    std::optional <uint32_t> count;
};

struct ship_requests::propulsion_t::items
{
    items (all_items const & value, size_t _index) :
        index(_index),
        count(value.count)
    {}
    
    size_t index;
    std::optional <uint32_t> count;
};

template <typename T>
requires (!pointer <T>)
std::map <int, size_t> index_mapping (std::vector <T> const & values)
{
    std::map <int, size_t> answer;
    for (size_t i = 0; i != values.size(); ++i)
        answer.insert({values[i].id, i});
    return answer;
}

template <typename T>
requires pointer <T>
std::map <int, size_t> index_mapping (std::vector <T> const & values)
{
    std::map <int, size_t> answer;
    for (size_t i = 0; i != values.size(); ++i)
        answer.insert({values[i]->id, i});
    return answer;
}

template <typename Object, std::vector <ship_requests::propulsion_t::items> (Object::* member), typename Items>
void ship_requests::propulsion_t::add_items
(
    std::vector <Object> & object_list,
    std::vector <Items> const & item_list,
    std::string_view from
)
{
    std::vector <all_items> object_use_items = request_to_db <all_items> (db, "select object_id, item_id, count from  ", from);
    std::map <int, size_t> object_mapping = index_mapping(object_list);
    std::map <int, size_t> item_mapping = index_mapping(item_list);
    for (all_items item_ : object_use_items)
    {
        std::map <int, size_t> ::iterator object = object_mapping.find(item_.object_id);
        std::map <int, size_t> ::iterator item = item_mapping.find(item_.item_id);
        if (object != object_mapping.end() && item != item_mapping.end())
            (object_list[object->second].*member).emplace_back(item_, item->second);
    }
}


struct ship_requests::propulsion_t::context
{
    context () = delete;
    context (propulsion_t & propulsion);

    context (context &&) = default;
    context (context const &) = delete;
    context & operator = (context &&) = default;
    context & operator = (context const &) = delete;
    
    struct boiling_type_t
    {
        boiling_type_t (pqxx::row const & value);
        
        enum boiling_type_mask : size_t
        {
            coil = 0,
            fuel_oil,
            total
        };

        static const constexpr std::array <std::string_view, total> heat_description =
        {{
            {"угольным"},
            {"нефтяным"}
        }};

        std::string description (print_context print) const;
            
        int id;
        std::optional <std::string> name;
        std::bitset <total> value;
        std::optional <double> temperature;
        std::optional <double> pressure;
        std::optional <double> heating_surface;
    };
    
    struct machine_type_t
    {
        machine_type_t (pqxx::row const & value);

        virtual ~machine_type_t () = default;
        virtual std::string description (print_context print) const = 0;

        int id;
        std::optional <std::string> name;
        std::optional <std::chrono::year_month_day> in_service;
    };
    
    std::vector <boiling_type_t> boiling_types;
    std::vector <std::unique_ptr <machine_type_t> > machine_types;
};


struct ship_requests::propulsion_t::print_context
{
    std::string_view tab = "    ";
    std::string_view new_line = "\n";
    std::string_view bold_begin = "";
    std::string_view bold_end = "";
};


struct ship_requests::propulsion_t::cilinder
{
    cilinder (pqxx::row const & value, size_t start_index = 0);

    std::optional <double> diameter; // mm
    std::optional <double> stroke;   // mm
};


struct ship_requests::propulsion_t::propulsion
{
    propulsion (pqxx::row const & value);

    propulsion () = delete;
    propulsion (propulsion const &) = default;
    propulsion (propulsion &&) = default;
    propulsion & operator = (propulsion const &) = default;
    propulsion & operator = (propulsion &&) = default;
    virtual ~propulsion () = default;
    
    virtual std::string description (print_context print, context const &) const
    {
        std::string answer;
        if (max_power)
            answer.append("мощность: ")
                  .append(print.bold_begin)
                  .append(to_string_10(*max_power))
                  .append("л.с.")
                  .append(print.bold_end)
                  .append(print.new_line);
        if (mass)
            answer.append("масса: ")
                  .append(to_string_10(*mass))
                  .append("кг")
                  .append(print.new_line);
        return answer;
    };
    
    int id;
    std::optional <double> mass; // kg
    std::optional <double> max_power; // horse power
    std::optional <std::chrono::year_month_day> in_service;
};


struct ship_requests::propulsion_t::diesel : propulsion
{
    diesel (pqxx::row const & value);

    diesel () = delete;
    diesel (diesel const &) = default;
    diesel (diesel &&) = default;
    diesel & operator = (diesel const &) = default;
    diesel & operator = (diesel &&) = default;
    virtual ~diesel () = default;

    virtual std::string description (print_context print, context const &) const;
    
    enum tact_t
    {
        two_tact,
        four_tact
    };

    std::string_view to_string (tact_t tact) const
    {
        switch (tact)
        {
        case two_tact:
            return "двухтактный ";
        case four_tact:
            return "четырехтактный ";
        }
    }
    
    std::optional <std::string> name;
    cilinder cilinders;
    std::optional <uint32_t> cilinder_count;
    std::optional <double> volume_of_engine; // liters
    std::optional <tact_t> tact;
};


struct ship_requests::propulsion_t::external_burn : propulsion
{
    external_burn (pqxx::row const & value);

    external_burn () = delete;
    external_burn (external_burn const &) = delete;
    external_burn (external_burn &&) = default;
    external_burn & operator = (external_burn const &) = delete;
    external_burn & operator = (external_burn &&) = default;
    virtual ~external_burn () = default;
    
    virtual std::string description (print_context print, context const & storage) const;
    
    std::vector <items> boiling_types;
    std::vector <items> machine_types;
};


struct ship_requests::propulsion_t::steam_turbine : context::machine_type_t
{
    steam_turbine (pqxx::row const & value);

    virtual ~steam_turbine () = default;
    
    virtual std::string description (print_context print) const
    {
        return description(print, "турбина");
    };

    std::string description (print_context print, std::string_view type) const;
    
    std::optional <double> rpm;
    std::optional <double> power;
    std::optional <uint32_t> stages;
};

struct ship_requests::propulsion_t::steam_turbine_reverse : steam_turbine
{
    steam_turbine_reverse (pqxx::row const & value);

    virtual ~steam_turbine_reverse () = default;
    
    virtual std::string description (print_context print) const
    {
        return steam_turbine::description(print, "турбина заднего хода");
    }
};

struct ship_requests::propulsion_t::steam_turbine_cruise : steam_turbine
{
    steam_turbine_cruise (pqxx::row const & value);

    virtual ~steam_turbine_cruise () = default;
    
    virtual std::string description (print_context print) const
    {
        return steam_turbine::description(print, "турбина крейсерского хода");
    }
};


struct ship_requests::propulsion_t::steam_machine : context::machine_type_t
{
    steam_machine (pqxx::row const & value);

    virtual ~steam_machine () = default;
    
    virtual std::string description (print_context print) const;
    
    struct cilinders_descr
    {
        cilinders_descr
        (
            cilinder _value,
            std::optional <uint32_t> _count
        ) :
            value(_value),
            count(_count)
        {}

        cilinder value;
        std::optional <uint32_t> count;
    };
    std::vector <cilinders_descr> cilinders;
};



struct ship_requests::propulsion_t::ship_propulsion
{
    ship_propulsion (pqxx::row const & value);
    
    int ship_id;
    int propulsion_id;
    uint32_t count;
    std::optional <std::chrono::year_month_day> date_from;
    std::optional <std::chrono::year_month_day> date_to;
};


#endif

