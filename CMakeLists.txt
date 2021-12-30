# Change this to your project name
set(PROJECT_NAME "schwrrring")

# cmake version to be used
cmake_minimum_required(VERSION 3.20)

# project name
project(schwrrring VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(tests)

# target
add_executable(schwrrring main.cpp)

# include
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

# external libs