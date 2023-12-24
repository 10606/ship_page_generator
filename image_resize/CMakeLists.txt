cmake_minimum_required(VERSION 3.0)

project(argc VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-Os --pedantic -flto -march=native -Wall -ftemplate-depth=10000 -std=c++20 -DNDEBUG"
)

set(compile_flags
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
     -fdiagnostics-color=always -fdiagnostics-generate-patch \
     -g --pedantic -Wall -ftemplate-depth=10000 -std=c++20"
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${release_flags} ${magick_flags}")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
    /usr/include/ImageMagick-7
)

add_executable(resize_image
    resize_image.cpp
)

include(FindPkgConfig)
pkg_check_modules(IMG_MAGIC "Magick++" REQUIRED)

target_link_libraries(resize_image PRIVATE ${IMG_MAGIC_LINK_LIBRARIES})
target_include_directories(resize_image PRIVATE ${IMG_MAGIC_INCLUDE_DIRS})
target_compile_options(resize_image PRIVATE ${IMG_MAGIC_CFLAGS})

