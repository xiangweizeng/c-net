
macro(cnet_add_operation class file)
    # WITH_LAYER_xxx option
    if(${ARGC} EQUAL 3)
        option(WITH_LAYER_${class} "build with layer ${class}" ${ARGV2})
    else()
        option(WITH_LAYER_${class} "build with layer ${class}" ON)
    endif()

    message(STATUS "WITH_LAYER_${class} = ${WITH_LAYER_${class}}")
    if(WITH_LAYER_${class})
        list(APPEND cnet_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/operation/${file}.c")
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

configure_file(operation_registry.c.in ${CMAKE_CURRENT_SOURCE_DIR}/operation_registry.c)
set(BASE_SRCS
        "operation.c"
        "network.c"
        "operation_benchmark.c"
        "quantize_s8.c"
        "operation_registry.c")

add_library(c-network ${BASE_SRCS} ${cnet_SRCS})
target_include_directories(c-network PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(c-network c-utils c-tensor)
