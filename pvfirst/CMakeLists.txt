cmake_minimum_required(VERSION 3.16)
project(pvfirst)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SIMGRID REQUIRED IMPORTED_TARGET simgrid)

file(GLOB_RECURSE SOURCES
    src/energy/*.cpp
    src/policy/*.cpp
    src/simulation/*.cpp
    src/sensors/*.cpp
    src/*.cpp
)

add_executable(pvfirst ${SOURCES})

target_link_libraries(pvfirst PRIVATE CURL::libcurl PkgConfig::SIMGRID)

# O executavel roda dentro da pasta build.
# Entao eu copio a plataforma do SimGrid para build/simgrid,
# assim o caminho "simgrid/platform.xml" continua funcionando sem gambiarra.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/simgrid)
configure_file(
    ${CMAKE_SOURCE_DIR}/simgrid/platform.xml
    ${CMAKE_BINARY_DIR}/simgrid/platform.xml
    COPYONLY
)