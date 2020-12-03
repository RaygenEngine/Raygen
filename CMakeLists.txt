cmake_minimum_required(VERSION 3.16)
project (Raygen)


# Dumb cmake bug? 
# For some reason include dir is cached somewhere and updating the sdk holds the old path.
# Maybe we should manually specify SDK anyway? CHECK:
# For whatever reason ${Vulkan_INCLUDE_DIR} has a value already (cached somewhere?).
# 
# Running find_package breaks this anyway. (Brings non existing include directories to subprojects)
# find_package(Vulkan REQUIRED)
if(NOT EXISTS ${Vulkan_INCLUDE_DIR})
if(WIN32)
message("Overwriting found vulkan directory because it does not exist.")
set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.2.154.1/Include")
set(Vulkan_LIBRARY "C:/VulkanSDK/1.2.154.1/Lib/Vulkan-1.lib")
endif()
endif()



set(OUTPUT_BIN_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(DEPENDENCIES_DIRECTORY "${CMAKE_SOURCE_DIR}/dependencies")

# main project and dependencies output location
# override at a lower level if need be
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIRECTORY})

add_subdirectory (dependencies)
add_subdirectory (source) 
add_subdirectory (tests) 

