

include_directories(include)
include_directories(${utils})
add_library(c-benchmark src/benchmark.c)

add_executable(test_benchmark test/vector_dot_int8_benchmark.c test/vector_dot_int8.c test/bench_example.c)
target_link_libraries(test_benchmark c-benchmark c-utils)

# Module definition
set(benchmark "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)


