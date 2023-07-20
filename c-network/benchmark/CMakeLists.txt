

include_directories(${benchmark})
add_executable(inner_product_benchmark inner_product.c inner_product_benchmark.c)
target_link_libraries(inner_product_benchmark c-benchmark c-utils)


