cmake_minimum_required(VERSION 3.0)

message("use clang")
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(ship_server VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-O3 --pedantic -flto -march=native -Wall -ftemplate-depth=10000 -std=c++20 -DNDEBUG"
)

set(compile_flags
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined -fdiagnostics-color=always \
     -g --pedantic -Wall -ftemplate-depth=10000 -std=c++20"
)

if(NOT DEFINED debug)
    set(debug FALSE)
endif()

if(debug)
    set(use_static FALSE)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${compile_flags}")
else()
    set(use_static TRUE)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${release_flags}")
endif()
message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
    server/
    server/allocators
    server/response
    server/response/ship_armament
    server/response/ship_armament/types
    server/response/ship/date_order
    server/response/armament
    server/response/armament/types
    server/response/aircraft
    server/response/ship
    get_from_postgres
)

message(${CMAKE_SOURCE_DIR})

find_library(libcxx c++)
if((libcxx OR not_use_cxx) AND EXISTS "${CMAKE_SOURCE_DIR}/libraries/libpqxx/CMakeLists.txt")
    if(not_use_cxx) 
        message("local pqxx found")
    else()
        message("local pqxx found -> use libc++")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wno-deprecated-declarations")
        set(stdlib c++ c++abi)
    endif()
    
    add_subdirectory("${CMAKE_SOURCE_DIR}/libraries/libpqxx" EXCLUDE_FROM_ALL)
    add_library(my_pqxx ALIAS pqxx)
    set_target_properties(pqxx PROPERTIES LINKER_LANGUAGE CXX)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libraries/libpqxx" APPEND PROPERTY "-DSKIP_BUILD_TEST=1")
else()
    message("libc++ or local pqxx not found -> use system")
    set(stdlib -static-libstdc++ -static-libgcc)
    include_directories(${CMAKE_SOURCE_DIR}/libraries/libpqxx/include)
    link_directories(${CMAKE_SOURCE_DIR}/libraries/libpqxx/src)
endif()

link_directories(${CMAKE_SOURCE_DIR}/libraries/openssl)
link_directories(${CMAKE_SOURCE_DIR}/libraries/postgresql/src/interfaces/libpq)
link_directories(${CMAKE_SOURCE_DIR}/libraries/postgresql/src/common)
link_directories(${CMAKE_SOURCE_DIR}/libraries/postgresql/src/port)

find_library(pqxx   NAMES libpqxx.a     libpqxx.so      pqxx    PATHS ${CMAKE_SOURCE_DIR}/libraries/libpqxx/src)
find_library(pqa    NAMES libpq.a       libpq.so        pq      PATHS ${CMAKE_SOURCE_DIR}/libraries/postgresql/src/interfaces/libpq)
find_library(ssla   NAMES libssl.a      libssl.so       ssl     PATHS ${CMAKE_SOURCE_DIR}/libraries/openssl)
find_library(crypto NAMES libcrypto.a   librcypto.so    crypto  PATHS ${CMAKE_SOURCE_DIR}/libraries/openssl)

if(pqxx)
    message("found pqxx ${pqxx}")
    set(use_static ${use_static} AND ${pqxx} MATCHES ".*\\.a")
endif()
if(pqa)
    message("found pq ${pqa}")
    set(use_static ${use_static} AND ${pqa} MATCHES ".*\\.a")
endif()
if(ssla)
    message("found ssl ${ssla}")
    set(use_static ${use_static} AND ${ssla} MATCHES ".*\\.a")
endif()
if(crypto)
    message("found crypto ${crypto}")
    set(use_static ${use_static} AND ${crypto} MATCHES ".*\\.a")
endif()
if(${use_static})
    message("static linkage")
    set(static_flags "-static")
endif()
    

add_executable(ship_server
    server/https_server.cpp
    server/driver/ssl_socket.cpp
    server/validate_path.cpp
    server/response/ship_armament/response_ship_armament.cpp
    server/response/ship_armament/ship_names.cpp
    server/response/ship_armament/types/common.cpp
    server/response/ship_armament/types/general.cpp
    server/response/ship_armament/types/guns.cpp
    server/response/ship_armament/types/torpedo.cpp
    server/response/ship_armament/types/throwers.cpp
    server/response/ship_armament/types/searchers.cpp
    server/response/ship_armament/types/catapult.cpp
    server/response/ship_armament/types/aircraft.cpp
    server/response/ship_armament/types/propulsion.cpp
    server/response/date_to_str.cpp
    server/response/parse_query.cpp
    server/response/armament/types/response_torpedo.cpp
    server/response/armament/types/response_guns.cpp
    server/response/armament/types/response_torpedo_tubes.cpp
    server/response/armament/types/response_mines_charges.cpp
    server/response/armament/types/response_catapult.cpp
    server/response/armament/types/response_searchers.cpp
    server/response/aircraft/response_aircraft.cpp
    server/response/documents.cpp
    server/response/menu.cpp
    server/response/menu_html.cpp
    server/response/html_view_pictures.cpp
    server/response/search.cpp
    server/response/ship/ship.cpp
    server/response/ship/date_order/nested_segments.cpp
    server/response/style.cpp
    get_from_postgres/parse_response.cpp
    get_from_postgres/ship_armament.cpp
    get_from_postgres/ship_armament_lt.cpp
    get_from_postgres/ship_event.cpp
    get_from_postgres/ship_info.cpp
    get_from_postgres/armament_info.cpp
    get_from_postgres/aircraft_info.cpp
    get_from_postgres/pictures.cpp
    get_from_postgres/ship_propulsion.cpp
    get_from_postgres/documents_info.cpp
)

target_link_libraries(ship_server
    # -s
    ${static_flags}
    pqxx
    pq
    pgcommon
    pgport
    ssl
    crypto
    ${stdlib}
)

