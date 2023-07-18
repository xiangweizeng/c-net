cmake_minimum_required(VERSION 3.1) 

project(netork-tools)

# for CMAKE_CXX_STANDARD
set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)

find_package(OpenMP)
if(NOT TARGET OpenMP::OpenMP_CXX AND (OpenMP_CXX_FOUND OR OPENMP_FOUND))
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(OpenCV_LIBS opencv_core
        opencv_highgui
        opencv_imgproc
        opencv_imgcodecs
        opencv_videoio)

add_subdirectory(external/ncnn)
add_subdirectory(nn-case)
add_subdirectory(quantize)