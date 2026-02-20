cmake_minimum_required(VERSION 3.16)
project(wrench_pvfirst)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

file(GLOB_RECURSE SOURCES
    src/energy/*.cpp
    src/policy/*.cpp
    src/simulation/*.cpp
    src/sensors/*.cpp
    src/*.cpp

)

add_executable(pvfirst ${SOURCES})

find_package(CURL REQUIRED)
target_link_libraries(pvfirst PRIVATE CURL::libcurl)

set(CMAKE_CXX_STANDARD 17)
