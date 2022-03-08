cmake_minimum_required(VERSION 3.0)

project(ship_server VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-Os --pedantic -flto -march=native -Wall -ftemplate-depth=10000 -std=c++20 -DNDEBUG"
)

set(compile_flags
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
     -fdiagnostics-color=always -fdiagnostics-generate-patch \
     -g --pedantic -Wall -ftemplate-depth=10000 -std=c++20"
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${release_flags}")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
    server/
    server/response
    server/response/ship_armament
    get_from_postgres
)

add_executable(ship_server
    server/https_server.cpp
    server/upgrade_to_https.cpp
    server/parse_query.cpp
    server/date_to_str.cpp
    server/response/ship_armament/guns.cpp
    server/response/ship_armament/torpedo.cpp
    server/response/ship_names.cpp
    get_from_postgres/parse_response.cpp
    get_from_postgres/ship_armament.cpp
    get_from_postgres/ship_event.cpp
    get_from_postgres/ship_info.cpp
    get_from_postgres/armament_info.cpp
    get_from_postgres/aircraft_info.cpp
)

target_link_libraries(ship_server
    pqxx
    pthread
    mongoose
    -static-libstdc++
    -static-libgcc
)

