#include "propulsion.h"



ship_requests::propulsion_t::cilinder::cilinder (pqxx::row const & value, size_t start_index) :
    diameter(value[start_index + 0].as <std::optional <double> > ()),
    stroke  (value[start_index + 1].as <std::optional <double> > ())
{}
    

ship_requests::propulsion_t::propulsion::propulsion (pqxx::row const & value) :
    id       (value[0].as <int> ()),
    mass     (value[1].as <std::optional <double> > ()),
    max_power(value[2].as <std::optional <double> > ()),
    in_service()
{
    std::optional <std::string> str_in_service = value[3].as <std::optional <std::string> > ();
    in_service = transform_optional(str_in_service, get_date);
}

std::vector <std::unique_ptr <ship_requests::propulsion_t::propulsion> > ship_requests::propulsion_t::get_propulsion (context const & storage, std::string_view where)
{
    std::vector <diesel> diesel_list = get_diesel(where);
    std::vector <external_burn> external_burn_list = get_external_burn (storage, where);
    std::vector <propulsion> unknown = request_to_db <propulsion> (db, "select id, mass, max_power, in_service from only propulsion ", where);
    
    std::vector <std::unique_ptr <propulsion> > answer;
    answer.reserve(diesel_list.size() + external_burn_list.size() + unknown.size());
    
    for (diesel & value : diesel_list)
        answer.push_back(std::make_unique <diesel> (std::move(value)));
    for (external_burn & value : external_burn_list)
        answer.push_back(std::make_unique <external_burn> (std::move(value)));
    for (propulsion & value : unknown)
        answer.push_back(std::make_unique <propulsion> (std::move(value)));
    
    return answer;
}


ship_requests::propulsion_t::diesel::diesel (pqxx::row const & value) :
    propulsion      (value),
    name            (value[4].as <std::optional <std::string> > ()),
    cilinders       (value, 5),
    cilinder_count  (value[7].as <std::optional <uint32_t> > ()),
    volume_of_engine(value[8].as <std::optional <double> > ()),
    tact(std::nullopt)
{
    std::optional <uint32_t> tact_value = value[8].as <uint32_t> ();
    if (tact_value)
    {
        if (*tact_value == 2)
            tact = two_tact;
        if (*tact_value == 4)
            tact = four_tact;
    }
}
    
std::string ship_requests::propulsion_t::diesel::description (context const & storage) const
{
    std::string answer;
    answer.append("x ");
    if (tact)
        answer.append(to_string(*tact));
    if (cilinder_count)
        answer.append(std::to_string(*cilinder_count))
              .append("-цилиндровый ");
    answer.append("дизель");
    if (name)
        answer.append(" типа")
              .append(*name);
    if (volume_of_engine)
        answer.append(" объемом ")
              .append(std::to_string(*volume_of_engine))
              .append("л");
    if (cilinders.diameter && cilinders.stroke)
        answer.append(" (\u2300")
              .append(std::to_string(*cilinders.diameter))
              .append("мм, \u2195")
              .append(std::to_string(*cilinders.stroke))
              .append("мм)");
    answer.append("\n")
          .append(propulsion::description(storage));
    return answer;
}

std::vector <ship_requests::propulsion_t::diesel> ship_requests::propulsion_t::get_diesel (std::string_view where)
{
    return request_to_db <diesel>
    (
        db,
        "select id, mass, max_power, in_service, name_en, \
                cilinder_diameter, cilinder_stroke, cilinder_count, \
                volume_of_engine, tact_value \
         from diesel_list ",
        where
    );
}


ship_requests::propulsion_t::context::boiling_type_t::boiling_type_t (pqxx::row const & value) :
    id          (value[0].as <int> ()),
    name        (value[1].as <std::optional <std::string> > ()),
    value       (value[2].as <uint32_t> ()),
    temperature (value[3].as <std::optional <double> > ()),
    pressure    (value[4].as <std::optional <double> > ())
{}
    

ship_requests::propulsion_t::context::machine_type_t::machine_type_t (pqxx::row const & value) :
    id  (value[0].as <int> ()),
    name(value[1].as <std::optional <std::string> > ())
{}

ship_requests::propulsion_t::context::context (propulsion_t & propulsion) :
    boiling_types(request_to_db <boiling_type_t> (propulsion.db, "select id, name_en, value, temperature, pressure from boiling_types", std::string_view()))
{
    std::vector <steam_turbine>         steam_turbines          = propulsion.get_steam_turbine();
    std::vector <steam_turbine_reverse> steam_turbines_reverse  = propulsion.get_steam_turbine_reverse();
    std::vector <steam_turbine_cruise>  steam_turbines_cruise   = propulsion.get_steam_turbine_cruise();
    std::vector <steam_machine>         steam_machines          = propulsion.get_steam_machine();
    
    for (steam_turbine & value : steam_turbines)
        machine_types.emplace_back(std::make_unique <steam_turbine> (std::move(value)));
    for (steam_turbine_reverse & value : steam_turbines_reverse)
        machine_types.emplace_back(std::make_unique <steam_turbine_reverse> (std::move(value)));
    for (steam_turbine_cruise & value : steam_turbines_cruise)
        machine_types.emplace_back(std::make_unique <steam_turbine_cruise> (std::move(value)));
    for (steam_machine & value : steam_machines)
        machine_types.emplace_back(std::make_unique <steam_machine> (std::move(value)));
}

std::vector <ship_requests::propulsion_t::steam_turbine> ship_requests::propulsion_t::get_steam_turbine (std::string_view where)
{
    return request_to_db <steam_turbine> (db, "select id, name_en, in_service from only steam_turbine ", where);
}

std::vector <ship_requests::propulsion_t::steam_turbine_reverse> ship_requests::propulsion_t::get_steam_turbine_reverse (std::string_view where)
{
    return request_to_db <steam_turbine_reverse> (db, "select id, name_en, in_service from steam_turbine_reverse ", where);
}

std::vector <ship_requests::propulsion_t::steam_turbine_cruise> ship_requests::propulsion_t::get_steam_turbine_cruise (std::string_view where)
{
    return request_to_db <steam_turbine_cruise> (db, "select id, name_en, in_service from steam_turbine_cruise ", where);
}

std::vector <ship_requests::propulsion_t::steam_machine> ship_requests::propulsion_t::get_steam_machine (std::string_view where)
{
    std::vector <steam_machine> answer =
        request_to_db <steam_machine> (db, "select id, name_en, in_service from steam_machine ", where);
    
    struct steam_machine_cilinders
    {
        steam_machine_cilinders (pqxx::row const & value) :
            machine_id(value[0].as <int> ()),
            value     (value, 1),
            count     (value[3].as <std::optional <uint32_t> > ())
        {}
    
        int machine_id;
        cilinder value;
        std::optional <uint32_t> count;
    };

    std::vector <steam_machine_cilinders> cilinders =
        request_to_db <steam_machine_cilinders> 
        (
            db, 
            "select machine_id, cilinder_diameter, cilinder_stroke, cilinder_count \
             from steam_machine_cilinders ", 
            where
        );

    std::map <int, size_t> object_mapping = index_mapping(answer);
    for (steam_machine_cilinders cilinder : cilinders)
    {
        std::map <int, size_t> ::iterator object = object_mapping.find(cilinder.machine_id);
        if (object != object_mapping.end())
            answer[object->second].cilinders.emplace_back(cilinder.value, cilinder.count);
    }
    
    return answer;
}


ship_requests::propulsion_t::external_burn::external_burn (pqxx::row const & value) :
    propulsion(value),
    boiling_types(),
    machine_types()
{}

std::vector <ship_requests::propulsion_t::external_burn> ship_requests::propulsion_t::get_external_burn (context const & storage, std::string_view where)
{
    std::vector <external_burn> answer = request_to_db <external_burn>
    (
        db,
        "select id, mass, max_power, in_service \
         from external_burn_list ",
        where
    );

    add_items <external_burn, &external_burn::boiling_types> (answer, storage.boiling_types, "external_burn_boiling");
    add_items <external_burn, &external_burn::machine_types> (answer, storage.machine_types, "external_burn_machines");

    return answer;
}

std::string ship_requests::propulsion_t::external_burn::description (context const & storage) const
{
    std::string answer;
    for (items boiling_type : boiling_types)
    {
        if (boiling_type.count)
        {
            answer.append(std::to_string(*boiling_type.count));
            if ((*boiling_type.count % 10 == 1) && (*boiling_type.count != 11))
                answer.append(" котел");
            else if ((*boiling_type.count % 10 <= 4) && ((*boiling_type.count / 10) % 10 != 1))
                answer.append(" котла"); // 12 "котлов" все-таки...
            else
                answer.append(" котлов");
        }
        else
            answer.append("котлы");
        answer.append(storage.boiling_types[boiling_type.index].description())
              .append("\n");
    }
    for (items machine_type : machine_types)
    {
        if (machine_type.count)
            answer.append(std::to_string(*machine_type.count))
                  .append("x ");
        answer.append(storage.machine_types[machine_type.index]->description())
              .append("\n");
    }
    answer.append(propulsion::description(storage));
    return answer;
}


std::string ship_requests::propulsion_t::context::boiling_type_t::description () const
{
    std::string answer;
    if (name)
        answer.append(" типа ")
              .append(*name);
    std::bitset <total> cur_value = value;
    if (cur_value.any())
    {
        answer += " с ";
        for (size_t i = 0; i != boiling_type_t::total; ++i)
        {
            if (cur_value[0])
            {
                answer += heat_description[i];
                if (cur_value.count() >= 3)
                    answer += ", ";
                else if (cur_value.count() == 2)
                    answer += " и ";
            }
            cur_value >>= 1;
        }
        answer += " отоплением";
    }
    if (temperature)
        answer.append(" ")
              .append(std::to_string(*temperature))
              .append("°C");
    if (pressure)
        answer.append(" ")
              .append(std::to_string(*pressure))
              .append("атм");
    return answer;
}


ship_requests::propulsion_t::steam_turbine::steam_turbine (pqxx::row const & value) :
    machine_type_t(value)
{}

ship_requests::propulsion_t::steam_turbine_reverse::steam_turbine_reverse (pqxx::row const & value) :
    steam_turbine(value)
{}


ship_requests::propulsion_t::steam_turbine_cruise::steam_turbine_cruise (pqxx::row const & value) :
    steam_turbine(value)
{}


ship_requests::propulsion_t::steam_machine::steam_machine (pqxx::row const & value) :
    machine_type_t(value)
{}
    
std::string ship_requests::propulsion_t::steam_machine::description () const
{
    std::string answer = "паровая машина";
    if (name)
        answer.append(*name);
    if (cilinders.empty()) {
        answer.append(" цилиндры: ");
        for (size_t i = 0; i != cilinders.size(); ++i)
        {
            cilinders_descr const & cilinder = cilinders[i];
            if (cilinder.count)
                answer.append(std::to_string(*cilinder.count))
                      .append("x ");
            if (cilinder.value.diameter)
                answer.append("D = ")
                      .append(std::to_string(*cilinder.value.diameter))
                      .append("mm ");
            if (cilinder.value.stroke)
                answer.append("h = ")
                      .append(std::to_string(*cilinder.value.stroke))
                      .append("mm ");
            if (i + 1 != cilinders.size())
                answer.append(", ");
        }
    }
    return answer;
}



ship_requests::propulsion_t::ship_propulsion::ship_propulsion (pqxx::row const & value) :
    ship_id         (value[0].as <int> ()),
    propulsion_id   (value[1].as <int> ()),
    count           (value[2].as <uint32_t> ()),
    date_from(),
    date_to()
{
    std::optional <std::string> str_date_from = value[3].as <std::optional <std::string> > ();
    std::optional <std::string> str_date_to   = value[4].as <std::optional <std::string> > ();
    date_from = transform_optional(str_date_from, get_date);
    date_to   = transform_optional(str_date_to,   get_date);
}

std::vector <ship_requests::propulsion_t::ship_propulsion> ship_requests::propulsion_t::get_ship_propulsion (std::string_view where)
{
    return request_to_db <ship_propulsion>
    (
        db,
        "select ship_id, propulsion_id, amount, \
                date_from, date_to \
         from ship_propulsion ",
        where
    );
}

