#ifndef SHIP_REQUESTS_H
#define SHIP_REQUESTS_H

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
        armament_info(&db),
        aircraft_info(&db),
        ship_event(&db)
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
        
        size_t count (std::string_view where);
        
        
        ship_event_t (ship_event_t &&) = delete;
        ship_event_t (ship_event_t const &) = delete;
        ship_event_t & operator = (ship_event_t &&) = delete;
        ship_event_t & operator = (ship_event_t const &) = delete;
        
    private:
        ship_database * db;
    };
    

    ship_database db;
    ship_info_t ship_info;
    ship_armament_t ship_armament;
    armament_info_t armament_info;
    aircraft_info_t aircraft_info;
    ship_event_t ship_event;
};


#endif

