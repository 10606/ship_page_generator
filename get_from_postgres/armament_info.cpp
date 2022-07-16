#include "ship_requests.h"
#include "armament_info.h"
#include "template_request.h"


ship_requests::armament_info_t::torpedo::torpedo (pqxx::row const & value) :
    id        (value[0].as <int> ()),
    torpedo_ru(value[1].as <std::optional <std::string> > ()),
    torpedo_en(value[2].as <std::optional <std::string> > ()),
    caliber (value[3].as <std::optional <double> > ()), /* мм */
    length  (value[4].as <std::optional <double> > ()), /* мм */
    speed   (value[5].as <std::optional <double> > ()), /* узлов */
    range   (value[6].as <std::optional <double> > ()), /* м */
    mass    (value[7].as <std::optional <double> > ()), /* кг */
    mass_ex (value[8].as <std::optional <double> > ()), /* кг */
    in_service()
{
    std::optional <std::string> str_in_service = value[9].as <std::optional <std::string> > ();
    in_service = transform_optional(str_in_service, get_date);
}

std::vector <ship_requests::armament_info_t::torpedo> ship_requests::armament_info_t::get_torpedo (std::string_view where)
{
    return request_to_db <torpedo>
    (
        db,
        std::string
        (
            "select id, name_ru, name_en, caliber, length, \
                    speed, range, mass, mass_ex, in_service \
             from torpedo_list "
        ) 
        .append(where)
    );
};


ship_requests::armament_info_t::torpedo_tubes::torpedo_tubes (pqxx::row const & value) :
    id      (value[0].as <int> ()),
    class_id(value[1].as <int> ()),
    tube_ru (value[2].as <std::optional <std::string> > ()),
    tube_en (value[3].as <std::optional <std::string> > ()),
    class_ru(value[4].as <std::optional <std::string> > ()),
    class_en(value[5].as <std::optional <std::string> > ()),
    caliber (value[6].as <std::optional <double> > ()),
    tubes_count(value[7].as <std::optional <uint32_t> > ()),
    in_service()
{
    std::optional <std::string> str_in_service = value[8].as <std::optional <std::string> > ();
    in_service = transform_optional(str_in_service, get_date);
}

std::vector <ship_requests::armament_info_t::torpedo_tubes> ship_requests::armament_info_t::get_torpedo_tubes (std::string_view where)
{
    return request_to_db <torpedo_tubes>
    (
        db,
        std::string
        (
            "select torpedo_tubes.id, torpedo_tubes.class_id, \
                    torpedo_tubes.name_ru, torpedo_tubes.name_en, \
                    gun_class.name_ru, gun_class.name_en, \
                    caliber, tubes_count, in_service \
             from torpedo_tubes \
             inner join gun_class on (torpedo_tubes.class_id = gun_class.id) "
        ) 
        +
        std::string(where)
    );
};


ship_requests::armament_info_t::classes::classes (pqxx::row const & value) :
    class_id (value[0].as <int> ()),
    parent_id(value[1].as <std::optional <int> > ()),
    class_ru (value[2].as <std::optional <std::string> > ()),
    class_en (value[3].as <std::optional <std::string> > ())
{}

std::vector <ship_requests::armament_info_t::classes> ship_requests::armament_info_t::get_classes (std::string_view where)
{
    return request_to_db <classes>
    (
        db,
        std::string ("select id, parent_id, name_ru, name_en from gun_class ") 
            .append(where)
    );
};


ship_requests::armament_info_t::list::list (pqxx::row const & value) :
    id      (value[0].as <int> ()),
    class_id(value[1].as <int> ()),
    gun_ru  (value[2].as <std::optional <std::string> > ()),
    gun_en  (value[3].as <std::optional <std::string> > ()),
    class_ru(value[4].as <std::optional <std::string> > ()),
    class_en(value[5].as <std::optional <std::string> > ()),
    caliber (value[6].as <std::optional <double> > ()),
    length  (value[7].as <std::optional <double> > ()),
    rate_of_fire(value[8].as <std::optional <double> > ()),
    effective_range(value[9].as <std::optional <double> > ()),
    mass    (value[10].as <std::optional <double> > ()),
    build_cnt(value[11].as <std::optional <uint32_t> > ()),
    in_service()
{
    std::optional <std::string> str_in_service = value[12].as <std::optional <std::string> > ();
    in_service = transform_optional(str_in_service, get_date);
}

std::vector <ship_requests::armament_info_t::list> ship_requests::armament_info_t::get_list (std::string_view where)
{
    return request_to_db <list>
    (
        db,
        std::string
        (
            "select gun_list.id, gun_list.class_id, \
                    gun_list.name_ru, gun_list.name_en, \
                    gun_class.name_ru, gun_class.name_en, \
                    caliber, length, rate_of_fire, effective_range, \
                    mass, build_cnt, in_service \
             from gun_list \
             inner join gun_class on (gun_list.class_id = gun_class.id) "
        ) 
        .append(where)
    );
};

    
ship_requests::armament_info_t::mount::mount (pqxx::row const & value) :
    id      (value[0].as <int> ()),
    gun_id  (value[1].as <int> ()),
    class_id(value[2].as <int> ()),
    mount_ru(value[3].as <std::optional <std::string> > ()),
    mount_en(value[4].as <std::optional <std::string> > ()),
    gun_ru  (value[5].as <std::optional <std::string> > ()),
    gun_en  (value[6].as <std::optional <std::string> > ()),
    class_ru(value[7].as <std::optional <std::string> > ()),
    class_en(value[8].as <std::optional <std::string> > ()),
    caliber (value[9].as <std::optional <double> > ()),
    length  (value[10].as <std::optional <double> > ()),
    rate_of_fire   (value[11].as <std::optional <double> > ()),
    effective_range(value[12].as <std::optional <double> > ()),
    gun_count(value[13].as <uint32_t> ()),
    angle    (value[14].as <std::optional <uint32_t> > ())
{}

std::vector <ship_requests::armament_info_t::mount> ship_requests::armament_info_t::get_mount (std::string_view where)
{
    return request_to_db <mount>
    (
        db,
        std::string
        (
            "select gun_mount.id, gun_mount.gun_id, gun_class.id, \
                    gun_mount.name_ru, gun_mount.name_en, \
                    gun_list.name_ru,  gun_list.name_en, \
                    gun_class.name_ru, gun_class.name_en, \
                    caliber, length, rate_of_fire, effective_range, \
                    gun_count, angle \
             from gun_mount \
             inner join gun_list on (gun_mount.gun_id = gun_list.id) \
             inner join gun_class on (gun_list.class_id = gun_class.id) "
        ) 
        .append(where)
    );
};


ship_requests::armament_info_t::mines_charges::mines_charges (pqxx::row const & value) :
    id      (value[0].as <int> ()),
    class_id(value[1].as <int> ()),
    mine_ru (value[2].as <std::optional <std::string> > ()),
    mine_en (value[3].as <std::optional <std::string> > ()),
    class_ru(value[4].as <std::optional <std::string> > ()),
    class_en(value[5].as <std::optional <std::string> > ()),
    size    (value[6].as <std::optional <double> > ()),
    mass    (value[7].as <std::optional <double> > ()),
    mass_ex (value[8].as <std::optional <double> > ()),
    in_service()
{
    std::optional <std::string> str_in_service = value[9].as <std::optional <std::string> > ();
    in_service = transform_optional(str_in_service, get_date);
}

std::vector <ship_requests::armament_info_t::mines_charges> ship_requests::armament_info_t::get_mines_charges (std::string_view where)
{
    return request_to_db <mines_charges>
    (
        db,
        std::string
        (
            "select mine_list.id, gun_class.id, \
                    mine_list.name_ru, mine_list.name_en, \
                    gun_class.name_ru, gun_class.name_en, \
                    size, mass, mass_ex \
             from mine_list \
             inner join gun_class on (mine_list.class_id = gun_class.id) "
        ) 
        .append(where)
    );
};


ship_requests::armament_info_t::throwers::throwers (pqxx::row const & value) :
    id        (value[0].as <int> ()),
    class_id  (value[1].as <int> ()),
    thrower_ru(value[2].as <std::optional <std::string> > ()),
    thrower_en(value[3].as <std::optional <std::string> > ()),
    class_ru  (value[4].as <std::optional <std::string> > ()),
    class_en  (value[5].as <std::optional <std::string> > ()),
    caliber   (value[6].as <std::optional <double> > ()),
    tubes_count(value[7].as <std::optional <uint32_t> > ()),
    in_service()
{
    std::optional <std::string> str_in_service = value[8].as <std::optional <std::string> > ();
    in_service = transform_optional(str_in_service, get_date);
}

std::vector <ship_requests::armament_info_t::throwers> ship_requests::armament_info_t::get_throwers (std::string_view where)
{
    return request_to_db <throwers>
    (
        db,
        std::string
        (
            "select throwers.id, gun_class.id, \
                    throwers.name_ru, throwers.name_en, \
                    gun_class.name_ru, gun_class.name_en, \
                    caliber, tubes_count, in_service \
             from throwers \
             inner join gun_class on (throwers.class_id = gun_class.id) "
        ) 
        .append(where)
    );
};


ship_requests::armament_info_t::catapult::catapult (pqxx::row const & value) :
    id          (value[0].as <int> ()),
    class_id    (value[1].as <int> ()),
    catapult_ru (value[2].as <std::optional <std::string> > ()),
    catapult_en (value[3].as <std::optional <std::string> > ()),
    class_ru    (value[4].as <std::optional <std::string> > ()),
    class_en    (value[5].as <std::optional <std::string> > ()),
    
    length      (value[6].as <std::optional <double> > ()),
    width       (value[7].as <std::optional <double> > ()),
    speed       (value[8].as <std::optional <double> > ()),
    launch_mass (value[9].as <std::optional <double> > ()),
    alleceration(value[10].as <std::optional <double> > ()),
    in_service()
{
    std::optional <std::string> str_in_service = value[11].as <std::optional <std::string> > ();
    in_service = transform_optional(str_in_service, get_date);
}

std::vector <ship_requests::armament_info_t::catapult> ship_requests::armament_info_t::get_catapult (std::string_view where)
{
    return request_to_db <catapult>
    (
        db,
        std::string
        (
            "select catapult.id, catapult_class.id, \
                    catapult.name_ru, catapult.name_en, \
                    catapult_class.name_ru, catapult_class.name_en, \
                    length, width, speed, launch_mass, \
                    alleceration, in_service \
             from catapult \
             inner join catapult_class on (catapult.class_id = catapult_class.id) "
        ) 
        .append(where)
    );
};


ship_requests::armament_info_t::searchers::searchers (pqxx::row const & value) :
    id         (value[0].as <int> ()),
    class_id   (value[1].as <int> ()),
    searcher_ru(value[2].as <std::optional <std::string> > ()),
    searcher_en(value[3].as <std::optional <std::string> > ()),
    class_ru   (value[4].as <std::optional <std::string> > ()),
    class_en   (value[5].as <std::optional <std::string> > ()),
    mass       (value[6].as <std::optional <double> > ()),
    frequency  (value[7].as <std::optional <double> > ()),
    power      (value[8].as <std::optional <double> > ()),
    build_cnt  (value[9].as <std::optional <uint32_t> > ()),
    in_service ()
{
    std::optional <std::string> str_in_service = value[10].as <std::optional <std::string> > ();
    in_service = transform_optional(str_in_service, get_date);
}

std::vector <ship_requests::armament_info_t::searchers> ship_requests::armament_info_t::get_searchers (std::string_view where)
{
    return request_to_db <searchers>
    (
        db,
        std::string
        (
            "select searchers.id, gun_class.id, \
                    searchers.name_ru, searchers.name_en, \
                    gun_class.name_ru, gun_class.name_en, \
                    mass, frequency, power, build_cnt, in_service \
             from searchers \
             inner join gun_class on (searchers.class_id = gun_class.id) "
        ) 
        .append(where)
    );
};


