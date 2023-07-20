
cmake_minimum_required(VERSION 3.0)
project(c-net C)

add_definitions(-Wall -Wno-unused-function -Ofast)
set(CMAKE_C_STANDARD 99)

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation Directory")
endif()
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE release CACHE STRING "Choose the type of build" FORCE)
endif()

add_subdirectory(c-utils)
add_subdirectory(c-benchmark)
add_subdirectory(c-tester)
add_subdirectory(c-tensor)
add_subdirectory(c-image)
add_subdirectory(c-network)
add_subdirectory(c-netend)
