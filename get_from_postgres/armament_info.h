#ifndef ARMAMENT_INFO_H
#define ARMAMENT_INFO_H

#include "ship_requests.h"


struct ship_requests::armament_info_t::torpedo
{
    torpedo (pqxx::row const & value);

    int torpedo_id;
    std::optional <std::string> torpedo_ru;
    std::optional <std::string> torpedo_en;
    std::optional <double> caliber; /* мм */
    std::optional <double> length;  /* мм */
    std::optional <double> speed;   /* узлов */
    std::optional <double> range;   /* м */
    std::optional <double> mass;    /* кг */
    std::optional <double> mass_ex; /* кг */
    std::optional <std::chrono::year_month_day> in_service;
};


struct ship_requests::armament_info_t::torpedo_tubes
{
    torpedo_tubes (pqxx::row const & value);
    
    int tube_id;
    int class_id;
    std::optional <std::string> tube_ru;
    std::optional <std::string> tube_en;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    
    std::optional <double> caliber; /* мм */
    std::optional <uint32_t> tubes_count;
    std::optional <std::chrono::year_month_day> in_service;
};


struct ship_requests::armament_info_t::classes
{
    classes (pqxx::row const & value);
    
    int class_id;
    std::optional <int> parent_id;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
};


struct ship_requests::armament_info_t::list
{
    list (pqxx::row const & value);
    
    int gun_id;
    int class_id;
    std::optional <std::string> gun_ru;
    std::optional <std::string> gun_en;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    
    std::optional <double> caliber; /* мм */
    std::optional <double> length;  /* калибров */
    std::optional <double> rate_of_fire;
    std::optional <double> effective_range; /* м */
    std::optional <double> mass; /* кг */
    std::optional <uint32_t> build_cnt;
    std::optional <std::chrono::year_month_day> in_service;
};


struct ship_requests::armament_info_t::mount
{
    mount (pqxx::row const & value);
    
    int mount_id;
    int gun_id;
    int class_id;
    std::optional <std::string> mount_ru;
    std::optional <std::string> mount_en;
    std::optional <std::string> gun_ru;
    std::optional <std::string> gun_en;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    
    std::optional <double> caliber; /* мм */
    std::optional <double> length;  /* калибров */
    std::optional <double> rate_of_fire;
    std::optional <double> effective_range; /* м */
    std::optional <uint32_t> gun_count;
    std::optional <double> angle;
};


struct ship_requests::armament_info_t::mines_charges
{
    mines_charges (pqxx::row const & value);
    
    int mine_id;
    int class_id;
    std::optional <std::string> mine_ru;
    std::optional <std::string> mine_en;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    
    std::optional <double> size; /* мм */
    std::optional <double> mass;
    std::optional <double> mass_ex;
    std::optional <std::chrono::year_month_day> in_service;
};
    

struct ship_requests::armament_info_t::throwers
{
    throwers (pqxx::row const & value);
    
    int thrower_id;
    int class_id;
    std::optional <std::string> thrower_ru;
    std::optional <std::string> thrower_en;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    
    std::optional <double> caliber; /* мм */
    std::optional <uint32_t> tubes_count;
    std::optional <std::chrono::year_month_day> in_service;
};


struct ship_requests::armament_info_t::catapult
{
    catapult (pqxx::row const & value);
    
    int catapult_id;
    int class_id;
    std::optional <std::string> catapult_ru;
    std::optional <std::string> catapult_en;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    
    std::optional <double> length;
    std::optional <double> width;
    std::optional <double> speed; /* м/с */
    std::optional <double> launch_mass; /* кг */
    std::optional <double> alleceration; /* g */
    std::optional <std::chrono::year_month_day> in_service;
};   


struct ship_requests::armament_info_t::searchers
{
    searchers (pqxx::row const & value);
    
    int searcher_id;
    int class_id;
    std::optional <std::string> searcher_ru;
    std::optional <std::string> searcher_en;
    std::optional <std::string> class_ru;
    std::optional <std::string> class_en;
    
    std::optional <double> mass;      /* кг */
    std::optional <double> frequency; /* МГц */
    std::optional <double> power;     /* кВт */
    std::optional <uint32_t> build_cnt;
    std::optional <std::chrono::year_month_day> in_service;
};


#endif

