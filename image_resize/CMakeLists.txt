cmake_minimum_required(VERSION 3.0)

project(argc VERSION 1.0 LANGUAGES C CXX)

# Magick++-config --cxxflags --cppflags
# Magick++-config --ldflags --libs
set(magick_flags
    "-fopenmp -DMAGICKCORE_HDRI_ENABLE=1 -DMAGICKCORE_QUANTUM_DEPTH=16"
)

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

target_link_libraries(resize_image
    Magick++-7.Q16HDRI
    MagickWand-7.Q16HDRI
    MagickCore-7.Q16HDRI
)

