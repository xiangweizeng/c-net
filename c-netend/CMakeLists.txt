

include_directories(include ${utils} ${tensor} ${image} ${network})
add_library(c-netend
        src/detection/object_box.c
        src/detection/mtcnn.c
        src/detection/retina_face.c
        src/detection/affine_matrix.c
        src/detection/yolov2.c
        src/detection/yolov3.c
        src/face/face_info.c
        src/face/arc_face.c
        src/face/mtcnn_face_detector.c
        src/lpr/plate_info.c
        src/lpr/lpr_recognizer.c
        src/lpr/lpc_recoginizer.c
        src/lpr/mtcnn_plate_detector.c
        )

target_link_libraries(c-netend c-network c-image)

# Module definition
set(netend "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)

add_subdirectory(examples)


