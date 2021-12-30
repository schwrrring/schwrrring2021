
cmake_minimum_required(VERSION 3.20)
project(test)
add_executable(test test.cpp)
add_subdirectory(libs/Catch2)
target_link_libraries(test PRIVATE Catch2::Catch2WithMain)
target_include_directories(test PUBLIC libs/Catch2/src/catch2)