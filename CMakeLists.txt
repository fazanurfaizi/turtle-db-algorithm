cmake_minimum_required(VERSION 3.16)
project(turtledb_algorithms LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

# Header-only library: everything under include/turtledb/ lives here.
add_library(turtledb INTERFACE)
target_include_directories(turtledb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(turtledb INTERFACE cxx_std_20)

enable_testing()
add_subdirectory(tests)
