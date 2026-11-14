cmake_minimum_required(VERSION 3.22.1)
project(the_hollow LANGUAGES CXX)

add_library(
    hollow
    SHARED
    hollow_jni.cpp
    game_core.cpp
)

target_compile_features(hollow PRIVATE cxx_std_17)

target_compile_options(
    hollow
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_include_directories(hollow PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
