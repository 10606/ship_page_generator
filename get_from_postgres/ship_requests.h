#ifndef SHIP_REQUESTS_H
#define SHIP_REQUESTS_H

#include <chrono>
#include <pqxx/pqxx>


struct ship_database
{
    ship_database () :
        conn("dbname=japan hostaddr=127.0.0.1")
    {};

    template <typename ... T>
    auto exec (T && ... args)
    {
        pqxx::read_transaction work(conn.conn);
        return work.exec(std::forward <T> (args) ...);
    }

private:
    struct conn_wrapper
    {
        template <typename ... T>
        conn_wrapper (T && ... args) :
            conn(std::forward <T> (args) ...)
        {
            if (!conn.is_open())
                throw std::runtime_error("can't connect");
            conn.set_client_encoding("UTF-8");
        }
        
        pqxx::connection conn;
    };
    
    conn_wrapper conn;
};


template <typename F, typename T>
std::optional <std::invoke_result_t <F, T> > transform_optional (std::optional <T> const & value, F && f)
{
    if (!value)
        return std::nullopt;
    return std::forward <F> (f)(value.value());
};


std::chrono::year_month_day get_date (std::string const & value);

std::string to_string (std::chrono::year_month_day const & value);

struct ship_requests
{
    ship_requests () :
        db(),
        ship_info(&db),
        ship_armament(&db),
        ship_armament_lt(&db),
        ship_propulsion(&db),
        armament_info(&db),
        aircraft_info(&db),
        ship_event(&db),
        pictures(&db)
    {}

    
    struct ship_info_t
    {
        ship_info_t (ship_database * _db) :
            db(_db)
        {}

        struct list;
        std::vector <list> get_list (std::string_view where = std::string_view());

        struct general;
        std::vector <general> get_general (std::string_view where);
 
        struct types;
        std::vector <types> get_types (std::string_view where = std::string_view());
 
        struct classes;
        std::vector <classes> get_classes (std::string_view where = std::string_view());
 
        struct sunk_dates;
        std::vector <sunk_dates> get_sunk_dates (std::string_view where = std::string_view());
        
 
        ship_info_t (ship_info_t &&) = delete;
        ship_info_t (ship_info_t const &) = delete;
        ship_info_t & operator = (ship_info_t &&) = delete;
        ship_info_t & operator = (ship_info_t const &) = delete;

    private:
        ship_database * db;
    };

    
    struct ship_armament_t
    {
        ship_armament_t (ship_database * _db) :
            db(_db)
        {}
    
        struct guns;
        std::vector <guns> get_guns (std::string_view where);

        struct torpedo_tubes;
        std::vector <torpedo_tubes> get_torpedo_tubes (std::string_view where);

        struct throwers;
        std::vector <throwers> get_throwers (std::string_view where);
        
        struct searchers;
        std::vector <searchers> get_searchers (std::string_view where);
        
        struct catapult;
        std::vector <catapult> get_catapult (std::string_view where);
        
        struct aircraft;
        std::vector <aircraft> get_aircraft (std::string_view where);
        
        
        ship_armament_t (ship_armament_t &&) = delete;
        ship_armament_t (ship_armament_t const &) = delete;
        ship_armament_t & operator = (ship_armament_t &&) = delete;
        ship_armament_t & operator = (ship_armament_t const &) = delete;

    private:
        ship_database * db;
    };
    

    struct ship_armament_lt_t
    {
        ship_armament_lt_t (ship_database * _db) :
            db(_db)
        {}
    
        struct guns;
        std::vector <guns> get_guns (std::string_view where);

        struct torpedo_tubes;
        std::vector <torpedo_tubes> get_torpedo_tubes (std::string_view where);

        struct throwers;
        std::vector <throwers> get_throwers (std::string_view where);
        
        struct searchers;
        std::vector <searchers> get_searchers (std::string_view where);
        
        struct catapult;
        std::vector <catapult> get_catapult (std::string_view where);
        
        struct aircraft;
        std::vector <aircraft> get_aircraft (std::string_view where);
        
        
        ship_armament_lt_t (ship_armament_lt_t &&) = delete;
        ship_armament_lt_t (ship_armament_lt_t const &) = delete;
        ship_armament_lt_t & operator = (ship_armament_lt_t &&) = delete;
        ship_armament_lt_t & operator = (ship_armament_lt_t const &) = delete;

    private:
        ship_database * db;
    };

    
    struct propulsion_t
    {
        propulsion_t (ship_database * _db) :
            db(_db)
        {}
        
        struct context;
        struct cilinder;

        struct propulsion;
        std::vector <std::unique_ptr <propulsion> > get_propulsion (context const & storage, std::string_view where = std::string_view());

        struct diesel;
        std::vector <diesel> get_diesel (std::string_view where = std::string_view());

        struct external_burn;
        std::vector <external_burn> get_external_burn (context const & storage, std::string_view where = std::string_view());

        // used for external burn
            struct steam_turbine;
            std::vector <steam_turbine> get_steam_turbine (std::string_view where = std::string_view());
     
            struct steam_turbine_reverse;
            std::vector <steam_turbine_reverse> get_steam_turbine_reverse (std::string_view where = std::string_view());
     
            struct steam_turbine_cruise;
            std::vector <steam_turbine_cruise> get_steam_turbine_cruise (std::string_view where = std::string_view());
     
            struct steam_machine;
            std::vector <steam_machine> get_steam_machine (std::string_view where = std::string_view());
            
        struct ship_propulsion;
        std::vector <ship_propulsion> get_ship_propulsion (std::string_view where = std::string_view());
 
        propulsion_t (propulsion_t &&) = delete;
        propulsion_t (propulsion_t const &) = delete;
        propulsion_t & operator = (propulsion_t &&) = delete;
        propulsion_t & operator = (propulsion_t const &) = delete;

        ship_database * db;
    private:

        struct all_items;
        struct items;
        
        template <typename Object, std::vector <items> (Object::* member), typename Items>
        void add_items
        (
            std::vector <Object> & objects_list,
            std::vector <Items> const & items_list,
            std::string_view from
        );
    };

    
    struct armament_info_t
    {
        armament_info_t (ship_database * _db) :
            db(_db)
        {}


        struct torpedo;
        std::vector <torpedo> get_torpedo (std::string_view where = std::string_view());
        
        struct torpedo_tubes;
        std::vector <torpedo_tubes> get_torpedo_tubes (std::string_view where = std::string_view());
        
        struct classes;
        std::vector <classes> get_classes (std::string_view where = std::string_view());
 
        struct list;
        std::vector <list> get_list (std::string_view where = std::string_view());
 
        struct mount;
        std::vector <mount> get_mount (std::string_view where = std::string_view());
 
        struct mines_charges;
        std::vector <mines_charges> get_mines_charges (std::string_view where = std::string_view());
 
        struct throwers;
        std::vector <throwers> get_throwers (std::string_view where = std::string_view());
 
        struct catapult;
        std::vector <catapult> get_catapult (std::string_view where = std::string_view());
 
        struct searchers;
        std::vector <searchers> get_searchers (std::string_view where = std::string_view());
 
        
        armament_info_t (armament_info_t &&) = delete;
        armament_info_t (armament_info_t const &) = delete;
        armament_info_t & operator = (armament_info_t &&) = delete;
        armament_info_t & operator = (armament_info_t const &) = delete;

    private:
        ship_database * db;
    };
    
    
    struct aircraft_info_t
    {
        aircraft_info_t (ship_database * _db) :
            db(_db)
        {}
 
        
        struct bombs;
        std::vector <bombs> get_bombs (std::string_view where = std::string_view());
 
        struct classes;
        std::vector <classes> get_classes (std::string_view where = std::string_view());
 
        struct types;
        std::vector <types> get_types (std::string_view where = std::string_view());
 
        struct list;
        std::vector <list> get_list (std::string_view where);
 
        struct guns;
        std::vector <guns> get_guns (std::string_view where = std::string_view());

        
        aircraft_info_t (aircraft_info_t &&) = delete;
        aircraft_info_t (aircraft_info_t const &) = delete;
        aircraft_info_t & operator = (aircraft_info_t &&) = delete;
        aircraft_info_t & operator = (aircraft_info_t const &) = delete;

    private:
        ship_database * db;
    };

    
    struct ship_event_t
    {
        ship_event_t (ship_database * _db) :
            db(_db)
        {}
        
        
        struct classes;
        std::vector <classes> get_classes (std::string_view where);
        
        struct event;
        std::vector <event> get_event (std::string_view where);
        
        struct event_lt;
        std::vector <event_lt> get_event_lt (std::string_view where = "");
        
        struct event_lt_descr;
        std::vector <event_lt_descr> get_event_lt_descr (std::string_view where = "");
        
        size_t count (std::string_view where);
        
        
        ship_event_t (ship_event_t &&) = delete;
        ship_event_t (ship_event_t const &) = delete;
        ship_event_t & operator = (ship_event_t &&) = delete;
        ship_event_t & operator = (ship_event_t const &) = delete;
        
    private:
        ship_database * db;
    };
    
    
    struct pictures_t
    {
        pictures_t (ship_database * _db) :
            db(_db)
        {}
        

        struct picture;
        std::vector <picture> get_ship (std::string_view where = "");
        std::vector <picture> get_aircraft (std::string_view where = "");
        std::vector <picture> get_gun (std::string_view where = "");
        std::vector <picture> get_searcher (std::string_view where = "");
        
        
        pictures_t (pictures_t &&) = delete;
        pictures_t (pictures_t const &) = delete;
        pictures_t & operator = (pictures_t &&) = delete;
        pictures_t & operator = (pictures_t const &) = delete;
        
    private:
        ship_database * db;
    };
    

    ship_database db;
    ship_info_t ship_info;
    ship_armament_t ship_armament;
    ship_armament_lt_t ship_armament_lt;
    propulsion_t ship_propulsion;
    armament_info_t armament_info;
    aircraft_info_t aircraft_info;
    ship_event_t ship_event;
    pictures_t pictures;
};


#endif

