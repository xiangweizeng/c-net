
cmake_minimum_required(VERSION 3.0)
project(cnet C)

add_definitions(-Wall -Wno-unused-function -Ofast)
#add_definitions(-Wall -Wno-unused-function)
set(CMAKE_C_STANDARD 99)

add_subdirectory(c-utils)
add_subdirectory(c-benchmark)
add_subdirectory(c-tester)
add_subdirectory(c-tensor)
add_subdirectory(c-image)
add_subdirectory(c-network)
add_subdirectory(c-netend)