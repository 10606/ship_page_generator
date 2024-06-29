cmake_minimum_required(VERSION 3.0)

project(allocators_test VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-Os --pedantic -flto -march=native -Wall -ftemplate-depth=10000 -std=c++20 -DNDEBUG"
)

set(compile_flags
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
     -fdiagnostics-color=always -fdiagnostics-generate-patch \
     -g --pedantic -Wall -ftemplate-depth=10000 -std=c++20"
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${compile_flags}")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
)

add_executable(allocators_test
    test_allocator.cpp
    allocator_avl.cpp 
    allocator_list.cpp
)

