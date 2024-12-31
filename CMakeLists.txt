cmake_minimum_required(VERSION 3.5.0)

project(HelloSDL VERSION 0.1.0 LANGUAGES C CXX)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
find_package(SDL3_image REQUIRED)

add_executable(HelloSDL new_main.cpp SDL3_wrapper.cpp stb_image.cpp mesh.cpp)

target_link_libraries(HelloSDL PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)
target_compile_options(HelloSDL PRIVATE -Wall)

if(TARGET SDL3::SDL3_test)
    target_link_libraries(HelloSDL PRIVATE SDL3::SDL3_test)
endif()

