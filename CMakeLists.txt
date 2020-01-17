cmake_minimum_required(VERSION 3.11)

project (Kaleido)

set(OUTPUT_BIN_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(DEPENDENCIES_DIRECTORY "${CMAKE_SOURCE_DIR}/dependencies")

# main project and dependencies output location
# override at a lower level if need be
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIRECTORY})

add_subdirectory (dependencies)
add_subdirectory (source) 

