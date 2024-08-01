cmake_minimum_required(VERSION 3.0)

project(benchmark VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-O3 --pedantic -flto -march=native -Wall -ftemplate-depth=10000 -std=c++20 -DNDEBUG"
)

set(compile_flags
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
     -fdiagnostics-color=always -fdiagnostics-generate-patch \
     -g --pedantic -Wall -ftemplate-depth=10000 -std=c++20"
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${release_flags}")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
    ../server
)

add_executable(benchmark
    benchmark.cpp
    ssl_socket.cpp
)

target_link_libraries(benchmark
    ssl
    crypto
)

