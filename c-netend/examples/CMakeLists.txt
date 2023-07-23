
cmake_minimum_required(VERSION 3.1)

project(netend-examples)
set(CMAKE_CPP_STANDARD CPP11)

find_package(OpenCV REQUIRED)

set(OpenCV_LIBS opencv_core
        opencv_highgui
        opencv_imgproc
        opencv_imgcodecs
        opencv_videoio)

add_executable(plate_retainface
        plate_retainface.cpp
        models/plate/plate-opt.c)
target_include_directories(plate_retainface PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(plate_retainface PRIVATE c-network c-image c-netend ${OpenCV_LIBS})

add_executable(lpr-demo
        lpr-demo.c
        models/plate/plate-opt.c
        models/plate/lpr-opt.c
        models/plate/lpc-opt.c)
target_include_directories(lpr-demo PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(lpr-demo PRIVATE c-network c-image c-netend m ${OpenCV_LIBS})

add_executable(plate_recognize
        plate_recognize.c
        models/plate/plate-opt.c
        models/plate/lpr-opt.c
        models/plate/lpc-opt.c)
target_include_directories(plate_recognize PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(plate_recognize PRIVATE c-network c-image c-netend m ${OpenCV_LIBS})

add_executable(face_retrainface
        face_retrainface.cpp
        models/face/face-opt.c)
target_include_directories(face_retrainface PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(face_retrainface PRIVATE c-network c-image c-netend m ${OpenCV_LIBS})

add_executable(face-demo
        face-demo.c
        models/face/mobilefacenet-opt.c)
target_include_directories(face-demo PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(face-demo PRIVATE c-network c-image c-netend m ${OpenCV_LIBS})

add_executable(network_verify
        network_verify.cpp
        models/plate/lpr-opt.c
        models/plate/lpc-opt.c
        models/plate/plate-opt.c
        models/face/mobilefacenet-opt.c
        )
target_include_directories(network_verify PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(network_verify PRIVATE c-network c-image c-netend ncnn ${OpenCV_LIBS})


