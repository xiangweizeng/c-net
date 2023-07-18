

include_directories(include)
add_library(c-tester src/tester.c src/unity.c)

add_executable(test_tester_example test/test_example.c)
target_link_libraries(test_tester_example c-tester)

# Module definition
set(tester "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)


