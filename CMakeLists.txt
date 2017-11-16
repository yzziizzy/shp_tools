cmake_minimum_required(VERSION 3.0)
project(shp_tools)


file(GLOB_RECURSE sources    src/*.cpp src/*.h)

add_executable(shptool ${sources})
target_include_directories(shptool PUBLIC src)

target_compile_options(shptool PUBLIC -std=c++11 )

target_link_libraries(shptool PUBLIC
  m
)

add_executable(unpackimg src/unpackimg.cpp)

target_compile_options(unpackimg PUBLIC -std=c++11 )

target_link_libraries(unpackimg PUBLIC
  m
)

