cmake_minimum_required(VERSION 3.0)

project(ship_tests VERSION 1.0 LANGUAGES C CXX)

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
    ../server/
    ../server/response
    ../server/response/ship
    ../server/response/ship/date_order
)

add_executable(tests
    main.cpp
    dekart_tree.cpp
    ../server/response/ship/date_order/nested_segments.cpp
    nested_segments.cpp
)

target_link_libraries(tests
    gtest
)

