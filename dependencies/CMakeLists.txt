# GLFW options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Glfw Docs")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Glfw Examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Glfw Test")
set(GLFW_INSTALL OFF CACHE BOOL "Glfw Install")
add_subdirectory(glfw)


add_subdirectory(spdlog)
add_subdirectory(imgui)
add_subdirectory(imguicolortextedit)
add_subdirectory(nativefiledialog)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_target_properties(
	glfw
	ImGui
	ImGuiColorTextEdit
PROPERTIES FOLDER forks)



set_target_properties(
	NativeFileDialog
	spdlog
PROPERTIES FOLDER ext_dependencies)
