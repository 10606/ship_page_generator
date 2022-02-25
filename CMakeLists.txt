cmake_minimum_required(VERSION 3.0)

project(ships VERSION 1.0 LANGUAGES C CXX)

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
    get_from_postgres/
)

add_executable(test
    test.cpp
    get_from_postgres/parse_response.cpp
    get_from_postgres/ship_armament.cpp
    get_from_postgres/ship_info.cpp
    get_from_postgres/armament_info.cpp
    get_from_postgres/aircraft_info.cpp
)

target_link_libraries(test
    pqxx
    -static-libstdc++
    -static-libgcc
)
