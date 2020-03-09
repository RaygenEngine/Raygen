project (Raygen-App)
project (Raygen)


set(SOURCE_FILES 
	App.cpp

	asset/AssetManager.cpp
	asset/PodLoaders.cpp
	asset/Serialization.cpp

	asset/util/DependenciesImpl.cpp
	asset/util/SpirvCompiler.cpp

	editor/Editor.cpp
	editor/AssetWindow.cpp
	editor/NodeContextActions.cpp
	editor/PropertyEditor.cpp
	editor/SceneSave.cpp
	editor/EdComponentWindows.cpp
	editor/EdUserSettings.cpp

	editor/utl/EdAssetUtils.cpp

	editor/windows/EdWindow.cpp
	editor/windows/WindowsRegistry.cpp
	
	editor/windows/general/EdAssetListWindow.cpp
	editor/windows/general/EdAssetsWindow.cpp
	editor/windows/general/EdConsoleWindow.cpp
	editor/windows/general/EdMiscWindow.cpp
	editor/windows/general/EdProfilerWindow.cpp
	
	editor/imgui/ImguiImpl.cpp
	editor/imgui/FileBrowser.cpp
	editor/imgui/ImGuizmo.cpp
	
	editor/misc/NativeFileBrowser.cpp

	engine/Engine.cpp
	engine/Object.cpp
	engine/Input.cpp
	engine/Logger.cpp
	engine/console/Console.cpp
	engine/profiler/Profiler.cpp
	engine/reflection/ReflectionDb.cpp
	
	platform/GlfwUtl.cpp

	renderer/GeometryPass.cpp
	renderer/DeferredPass.cpp
	renderer/EditorPass.cpp
	renderer/VulkanLayer.cpp
	renderer/Instance.cpp
	renderer/PhysicalDevice.cpp
	renderer/Device.cpp
	renderer/Swapchain.cpp
	renderer/Model.cpp
	renderer/Texture.cpp
	renderer/Image.cpp
	renderer/Material.cpp
	

	world/World.cpp
	world/NodeFactory.cpp
	
	world/nodes/Node.cpp
	world/nodes/RootNode.cpp
	world/nodes/camera/CameraNode.cpp
	world/nodes/camera/WindowCameraNode.cpp
	world/nodes/camera/EditorCameraNode.cpp
	world/nodes/geometry/GeometryNode.cpp
	world/nodes/light/PunctualLightNode.cpp
	world/nodes/light/DirectionalLightNode.cpp
	world/nodes/light/SpotLightNode.cpp
	world/nodes/light/LightNode.cpp
	world/nodes/light/AmbientNode.cpp
	world/nodes/user/FreeformUserNode.cpp
	world/nodes/user/UserNode.cpp
	world/nodes/user/FlyingUserNode.cpp
)

file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "*.h")

set(APP_SOURCES
	AppMain.cpp
)

# add pch for msvc 
if(MSVC)
	set(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>_pch/pch.pch")

	set_source_files_properties(pch.cpp
								PROPERTIES 
								COMPILE_FLAGS "/Yc\"pch.h\" /Fp\"${PrecompiledBinary}\""
								OBJECT_OUTPUTS "${PrecompiledBinary}")
	
	set_source_files_properties(${SOURCE_FILES}
								PROPERTIES 
								COMPILE_FLAGS "/Yu\"pch.h\" /FI\"pch.h\" /Fp\"${PrecompiledBinary}\""
							    OBJECT_DEPENDS "${PrecompiledBinary}")  
										   
	list(APPEND SOURCE_FILES pch.cpp)
	list(APPEND APP_SOURCES platform/windows/resources/WinResource.rc)
endif()

add_library(Raygen ${SOURCE_FILES} ${HEADER_FILES})
add_executable(Raygen-App ${APP_SOURCES})
 
if(MSVC)
	set_property(TARGET Raygen-App PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

# Vulkan
find_package(Vulkan REQUIRED)

# 3rd party libraries
target_link_libraries(Raygen
	NativeFileDialog
	XInput.lib
	Vulkan::Vulkan
	ImGui
	spdlog
	# shader compiler
	SPIRV
	glslang-default-resource-limits
	#end of shader compiler
	glfw
	${GLFW_LIBRARIES}
)

target_link_libraries(Raygen-App
	Raygen
)

# instruct glfw to globaly include vulkan headers
add_compile_definitions(GLFW_INCLUDE_VULKAN)
add_compile_definitions(GLFW_INCLUDE_NONE)

# dependencies (build order)
add_dependencies(Raygen 
	ImGui
	glfw
	spdlog
)

target_include_directories(Raygen PUBLIC ${INCLUDE_DEPENDENCIES_DIRECTORY})
target_include_directories(Raygen PUBLIC ${INCLUDE_DEPENDENCIES_DIRECTORY}/nlohmann)
							  
target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY})
target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY}/glm)
target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY}/spdlog/include)
target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY}/imgui)
target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY}/magic_enum/include)
target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY}/nlohmann/include)
target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY}/tinygltf)
target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY}/imfilebrowser)
target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY}/GLFW/include)
target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY}/cereal/include)
target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY}/nativefiledialog/src/include)


target_include_directories(Raygen PUBLIC ${Vulkan_INCLUDE_DIR}/../glslang)


target_include_directories(Raygen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(Raygen PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#include_directories(RAYGEN_INCLUDE_DIRS_LOCAL)

# enforce cpp20
set_property(TARGET Raygen Raygen-App PROPERTY CXX_STANDARD 20)
set_property(TARGET Raygen Raygen-App PROPERTY CXX_STANDARD_REQUIRED ON)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX ${PROJECT_NAME} FILES ${HEADER_FILES} ${SOURCE_FILES})

#definitions (WINDOWS)
add_compile_definitions(NOMINMAX NODRAWTEXT NOOPENFILE) # TODO:

# multi-core compilation
if(MSVC)
	target_compile_options(Raygen PRIVATE "/MP")
	# unsafe strcpy used by some dependencies
	add_definitions(-D_CRT_SECURE_NO_WARNINGS) # CHECK:
endif()


set(RAYGEN_INCLUDE_DIRS ${RAYGEN_INCLUDE_DIRS_LOCAL}
    CACHE INTERNAL "Engine Include Directories" FORCE)
