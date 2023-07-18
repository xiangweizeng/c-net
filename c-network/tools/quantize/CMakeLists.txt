cmake_minimum_required(VERSION 3.1) 

# for CMAKE_CXX_STANDARD
set(CMAKE_CXX_STANDARD 11)

add_executable(calibration calibration.cpp quantize_data.cpp)
target_include_directories(calibration PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(calibration PRIVATE ncnn ${OpenCV_LIBS})

if(OpenMP_CXX_FOUND)
    if(NCNN_CMAKE_VERBOSE)
        message("Building with OpenMP")
    endif()
    target_link_libraries(calibration PUBLIC OpenMP::OpenMP_CXX)
endif()