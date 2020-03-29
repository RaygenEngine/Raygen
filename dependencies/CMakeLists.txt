
# GLFW options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Glfw Docs")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Glfw Examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Glfw Test")
set(GLFW_INSTALL OFF CACHE BOOL "Glfw Install")
set(GLFW_VULKAN_STATIC ON CACHE BOOL "Glfw Install")
add_subdirectory(glfw)


add_subdirectory(spdlog)
add_subdirectory(imgui)
add_subdirectory(nativefiledialog)



# SPIRV-CROSS default config
set(SPIRV_CROSS_CLI OFF CACHE BOOL "SPIRV-Cross")
set(SPIRV_CROSS_ENABLE_CPP OFF CACHE BOOL "SPIRV-Cross")
set(SPIRV_CROSS_ENABLE_C_API OFF CACHE BOOL "SPIRV-Cross")
set(SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "SPIRV-Cross")
set(SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "SPIRV-Cross")
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "SPIRV-Cross")
set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE BOOL "SPIRV-Cross")
add_subdirectory(spirv-cross)
set(RAYGEN_SPIRV_CROSS_MODULES spirv-cross-core spirv-cross-glsl spirv-cross-reflect PARENT_SCOPE)


# Vulkan
find_package(Vulkan REQUIRED)
add_subdirectory(${Vulkan_INCLUDE_DIR}/../glslang glslang/ EXCLUDE_FROM_ALL)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_target_properties(
	glfw
	ImGui
PROPERTIES FOLDER forks)



set_target_properties(
	NativeFileDialog
	spdlog
	glslang
	HLSL
	core_tables
	extinst_tables
	enum_string_mapping
	SPIRV-Tools
	SPIRV-Tools-opt
	OGLCompiler
	OSDependent
	SPIRV
	spirv-tools-header-DebugInfo
	spirv-tools-header-OpenCLDebugInfo100
	spirv-cross-core
	spirv-cross-glsl
	spirv-cross-reflect
	spv-tools-debuginfo
	spv-tools-spv-amd-gs
	spv-tools-spv-amd-sb
	spv-tools-spv-amd-sevp
	spv-tools-spv-amd-stm
	spv-tools-cldi100
	glslang-default-resource-limits
PROPERTIES FOLDER ext_dependencies)
