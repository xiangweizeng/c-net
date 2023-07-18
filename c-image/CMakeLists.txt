

include_directories(include ${utils} ${tensor})
add_compile_definitions(STB_IMAGE)
add_library(c-image src/imgproc.c src/tensor_pixel.c)

# Module definition
set(image "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)


