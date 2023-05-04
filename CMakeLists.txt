cmake_minimum_required(VERSION 3.0)

project(ship_server VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-O3 --pedantic -flto -march=native -Wall -ftemplate-depth=10000 -std=c++20 -DNDEBUG"
)

set(compile_flags
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined -fdiagnostics-color=always \
     -g --pedantic -Wall -ftemplate-depth=10000 -std=c++20"
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${release_flags} -DMG_ENABLE_OPENSSL=1")
set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS}   -O3 -flto -march=native \
                                         -DNDEBUG -DMG_ENABLE_OPENSSL=1 -DMG_ENABLE_IPV6=1")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

message("use clang")
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

include_directories(
    server/
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
include_directories(${CMAKE_SOURCE_DIR}/libraries/libpqxx/include)
link_directories(${CMAKE_SOURCE_DIR}/libraries/libpqxx/src)
link_directories(${CMAKE_SOURCE_DIR}/libraries/openssl)
link_directories(${CMAKE_SOURCE_DIR}/libraries/postgresql/src/interfaces/libpq)
link_directories(${CMAKE_SOURCE_DIR}/libraries/postgresql/src/common)
link_directories(${CMAKE_SOURCE_DIR}/libraries/postgresql/src/port)

find_library(pqxx libpqxx.a)
find_library(ssl libssl.a)
find_library(crypto libcrypto.a)


add_executable(ship_server
    server/https_server.cpp
    server/driver/ssl_socket.cpp
    server/date_to_str.cpp
    server/validate_path.cpp
    server/response/ship_armament/response_ship_armament.cpp
    server/response/ship_armament/types/common.cpp
    server/response/ship_armament/types/general.cpp
    server/response/ship_armament/types/guns.cpp
    server/response/ship_armament/types/torpedo.cpp
    server/response/ship_armament/types/throwers.cpp
    server/response/ship_armament/types/searchers.cpp
    server/response/ship_armament/types/catapult.cpp
    server/response/ship_armament/types/aircraft.cpp
    server/response/ship_armament/types/propulsion.cpp
    server/response/parse_query.cpp
    server/response/armament/types/response_torpedo.cpp
    server/response/armament/types/response_guns.cpp
    server/response/armament/types/response_torpedo_tubes.cpp
    server/response/armament/types/response_mines_charges.cpp
    server/response/armament/types/response_catapult.cpp
    server/response/armament/types/response_searchers.cpp
    server/response/aircraft/response_aircraft.cpp
    server/response/ship_names.cpp
    server/response/menu.cpp
    server/response/menu_html.cpp
    server/response/search.cpp
    server/response/ship/ship.cpp
    server/response/ship/date_order/nested_segments.cpp
    get_from_postgres/parse_response.cpp
    get_from_postgres/ship_armament.cpp
    get_from_postgres/ship_armament_lt.cpp
    get_from_postgres/ship_event.cpp
    get_from_postgres/ship_info.cpp
    get_from_postgres/armament_info.cpp
    get_from_postgres/aircraft_info.cpp
    get_from_postgres/pictures.cpp
    get_from_postgres/ship_propulsion.cpp
)

add_executable(tests
    test/main.cpp
    test/dekart_tree.cpp
    server/response/ship/date_order/nested_segments.cpp
    test/nested_segments.cpp
)

target_link_libraries(tests
    gtest
)


target_link_libraries(ship_server
    -static
    pqxx
    pq
    pgcommon
    pgport
    ssl
    crypto
    -static-libstdc++
    -static-libgcc
)

