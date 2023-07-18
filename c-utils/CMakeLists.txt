
include_directories(include)

set(UTILS_SRCS
        src/allocator.c
        src/container_linked_list.c
        src/container_vector.c
        src/threadpool-pthreads.c
        src/option.c
        src/ncx-pool/ncx_slab.c
        src/memory_cached.c)
add_library(c-utils ${UTILS_SRCS})


# Module definition
set(utils "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
