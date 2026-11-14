cmake_minimum_required(VERSION 3.16)
project(the_hollow_native_tests LANGUAGES CXX)

enable_testing()

add_executable(
    hollow_core_tests
    ../../app/src/main/cpp/game_core.cpp
    game_core_tests.cpp
)

target_compile_features(hollow_core_tests PRIVATE cxx_std_17)
target_include_directories(
    hollow_core_tests
    PRIVATE
    ../../app/src/main/cpp
)

add_test(NAME hollow_core_tests COMMAND hollow_core_tests)
