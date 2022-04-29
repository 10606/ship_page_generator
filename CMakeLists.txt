cmake_minimum_required(VERSION 3.0)

project(ship_server VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-O3 --pedantic -flto -march=native -Wall -ftemplate-depth=10000 -std=c++20 -DNDEBUG"
)

set(compile_flags
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
     -fdiagnostics-color=always -fdiagnostics-generate-patch \
     -g --pedantic -Wall -ftemplate-depth=10000 -std=c++20"
)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${release_flags} -DMG_ENABLE_OPENSSL=1")
set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS}  -Os -flto -march=native \
                                        -DNDEBUG -DMG_ENABLE_OPENSSL=1")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
    server/
    server/response
    server/response/ship_armament
    server/response/date_order
    server/response/armament
    server/response/aircraft
    get_from_postgres
)

add_executable(ship_server
    server/https_server.cpp
    server/upgrade_to_https.cpp
    server/parse_query.cpp
    server/date_to_str.cpp
    server/validate_path.cpp
    server/mongoose.c
    server/response/ship_armament/response_ship_armament.cpp
    server/response/ship_armament/general.cpp
    server/response/ship_armament/guns.cpp
    server/response/ship_armament/torpedo.cpp
    server/response/ship_armament/throwers.cpp
    server/response/ship_armament/searchers.cpp
    server/response/ship_armament/catapult.cpp
    server/response/ship_armament/aircraft.cpp
    server/response/armament/parse_query.cpp
    server/response/armament/response_torpedo.cpp
    server/response/armament/response_guns.cpp
    server/response/armament/response_torpedo_tubes.cpp
    server/response/armament/response_mines_charges.cpp
    server/response/armament/response_catapult.cpp
    server/response/armament/response_searchers.cpp
    server/response/aircraft/response_aircraft.cpp
    server/response/ship_names.cpp
    server/response/menu.cpp
    get_from_postgres/parse_response.cpp
    get_from_postgres/ship_armament.cpp
    get_from_postgres/ship_armament_lt.cpp
    get_from_postgres/ship_event.cpp
    get_from_postgres/ship_info.cpp
    get_from_postgres/armament_info.cpp
    get_from_postgres/aircraft_info.cpp
)

add_executable(tests
    test/main.cpp
    test/dekart_tree.cpp
    server/response/date_order/nested_segments.cpp
    test/nested_segments.cpp
)

target_link_libraries(tests
    gtest
)

target_link_libraries(ship_server
    pqxx
    pthread
    ssl
    crypto
    -static-libstdc++
    -static-libgcc
)

