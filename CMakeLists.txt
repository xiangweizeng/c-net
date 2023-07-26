
option(ENABLE_STB_IMAGE "enable stb image" OFF)
option(ENABLE_ESP32 "enable esp32 build" ON)
option(ENABLE_POSIX_PORTS "enable posix ports supported" OFF)
option(ENABLE_DEBUG_MEMORY "enable debug memory allocation" ON)

macro(cnet_add_operation class file)
    # WITH_LAYER_xxx option
    if(${ARGC} EQUAL 3)
        option(WITH_LAYER_${class} "build with layer ${class}" ${ARGV2})
    else()
        option(WITH_LAYER_${class} "build with layer ${class}" ON)
    endif()

    message(STATUS "WITH_LAYER_${class} = ${WITH_LAYER_${class}}")
    if(WITH_LAYER_${class})
        list(APPEND OPERATION_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/c-network/src/operation/${file}.c")
    endif()

    # generate operation_declaration and operation_registry file
    if(WITH_LAYER_${class})
        set(operation_names     "${operation_names}\t\"${class}\",\n")
        set(operation_creators  "${operation_creators}/// Creator for ${class}\nDEFINE_OPERATION_CREATOR(${class});\n\n")
        set(operation_creator   "${operation_creator}\t${class}_operation_creator, // ${class}\n")
    else()
        set(operation_names     "${operation_names}\t\"\",\n")
        set(operation_creator   "${operation_creator}\t0/*${class}_operation_creator*/, // ${class}\n")
    endif()

endmacro()

set(UTILS_SRCS
        c-utils/src/allocator.c
        c-utils/src/container_linked_list.c
        c-utils/src/container_vector.c
        c-utils/src/threadpool-pthreads.c
        c-utils/src/option.c
        c-utils/src/ncx-pool/ncx_slab.c
        c-utils/src/memory_cached.c
        )

set(TESTER_SRCS
        c-tester/src/tester.c
        c-tester/src/unity.c
        )

set(BENCH_MARK_SRCS
        c-benchmark/src/benchmark.c
        )

set(TENSOR_SRCS
        c-tensor/src/tensor.c
        c-tensor/src/tensor_softmax.c
        c-tensor/src/tensor_padding.c
        c-tensor/src/tensor_cut_border.c
        c-tensor/src/tensor_resize_bilinear.c
        c-tensor/src/tensor_resize_bicubic.c
        )

cnet_add_operation(convolution convolution ON)
cnet_add_operation(pooling pooling ON)
cnet_add_operation(padding padding ON)
cnet_add_operation(crop crop ON)
cnet_add_operation(inner_product inner_product ON)
cnet_add_operation(activation activation ON)
cnet_add_operation(batch_norm batch_norm ON)
cnet_add_operation(slice slice ON)
cnet_add_operation(concat concat ON)
cnet_add_operation(reshape reshape ON)
cnet_add_operation(sigmoid sigmoid ON)
cnet_add_operation(softmax softmax ON)
cnet_add_operation(permute permute ON)
cnet_add_operation(unary unary ON)
cnet_add_operation(binary binary ON)
cnet_add_operation(upsample upsample ON)
cnet_add_operation(prelu prelu ON)
cnet_add_operation(memory_data memory_data ON)
cnet_add_operation(shuffle_channel shuffle_channel ON)

set(NETWORK_SRCS
        c-network/src/operation.c
        c-network/src/network.c
        c-network/src/operation_benchmark.c
        c-network/src/quantize16.c
        c-network/src/operation_registry.c
        ${OPERATION_SRCS}
        )

set(NETEND_SRCS
        c-netend/src/detection/object_box.c
        c-netend/src/detection/mtcnn.c
        c-netend/src/detection/retina_face.c
        c-netend/src/detection/affine_matrix.c
        c-netend/src/detection/yolov2.c
        c-netend/src/detection/yolov3.c
        c-netend/src/face/face_info.c
        c-netend/src/face/arc_face.c
        c-netend/src/face/mtcnn_face_detector.c
        c-netend/src/lpr/plate_info.c
        c-netend/src/lpr/lpr_recognizer.c
        c-netend/src/lpr/lpc_recoginizer.c
        c-netend/src/lpr/mtcnn_plate_detector.c
        )

set(IMAGE_SRCS
        c-image/src/imgproc.c
        c-image/src/tensor_pixel.c
        )

set(COMPONENT_SRCS
        ${UTILS_SRCS}
        ${TESTER_SRCS}
        ${BENCH_MARK_SRCS}
        ${TENSOR_SRCS}
        ${NETWORK_SRCS}
        ${NETEND_SRCS}
        ${IMAGE_SRCS}
        )

set(COMPONENT_ADD_INCLUDEDIRS
        c-utils/include
        c-tester/include
        c-benchmark/include
        c-tensor/include
        c-network/include
        c-netend/include
        c-image/include
        )

set(COMPONENT_ADD_INCLUDEDIRS
    c-utils/include
    c-tester/include
    c-benchmark/include
    c-tensor/include
    c-network/include
    c-netend/include
    c-image/include
    )
  
register_component()
configure_file(c-network/src/operation_registry.c.in 
            ${CMAKE_CURRENT_SOURCE_DIR}/c-network/src/operation_registry.c)
add_definitions(-DESP32)
target_compile_options(${COMPONENT_LIB} PRIVATE  -O2 -Wno-unused-function)
