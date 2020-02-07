# GLFW options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Glfw Docs")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Glfw Examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Glfw Test")
set(GLFW_INSTALL OFF CACHE BOOL "Glfw Install")
set(GLFW_VULKAN_STATIC ON CACHE BOOL "Glfw Install")

add_subdirectory(glfw)
add_subdirectory(spdlog)
add_subdirectory(imgui)
