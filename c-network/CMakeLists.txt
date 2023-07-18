
# Modules
include_directories(include ${utils} ${tensor} ${image})
add_subdirectory("src")

# Module definition
set(network "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)

add_subdirectory("tools")