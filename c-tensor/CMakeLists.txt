

include_directories(include)
include_directories(${utils})
add_library(c-tensor
        src/tensor.c
        src/tensor_softmax.c
        src/tensor_padding.c
        src/tensor_cut_border.c
        src/tensor_resize_bilinear.c
        src/tensor_resize_bicubic.c
        )

# Module definition
set(tensor "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)


