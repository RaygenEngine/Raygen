project (Raygen)

set(SOURCE_FILES 
	main.cpp
	AppBase.cpp

	asset/AssetManager.cpp
	asset/PodLoaders.cpp

	asset/util/DependenciesImpl.cpp

	editor/Editor.cpp
	editor/AssetWindow.cpp
	editor/NodeContextActions.cpp
	editor/PropertyEditor.cpp
	editor/SceneSave.cpp

	editor/imgui/ImguiImpl.cpp
    editor/imgui/ImguiDemo.cpp
	
	platform/Window.cpp
	platform/GlfwUtil.cpp

	platform/windows/Win32Window.cpp
	
	renderer/ObserverRenderer.cpp
	renderer/renderers/vulkan/VkRendererBase.cpp

	system/Engine.cpp
	system/Input.cpp
	system/Logger.cpp


	world/World.cpp
	world/NodeFactory.cpp
	
	world/nodes/Node.cpp
	world/nodes/RootNode.cpp
	world/nodes/vr/OVRNode.cpp
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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app/CMakeLists.txt" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app/App.h")
	set(APP_DIR "app")
	message(STATUS "Detected app header at '${CMAKE_CURRENT_SOURCE_DIR}/app'. Using custom app files.") 
else()
	set(APP_DIR "app-default")
	message(STATUS "No specific app files detected. Using app-default.")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${APP_DIR})
include(${APP_DIR}/CMakeLists.txt)


# add pch for msvc 
if(MSVC)

	set(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>_pch/pch.pch")

	set_source_files_properties(pch/pch.cpp
								PROPERTIES 
								COMPILE_FLAGS "/Yc\"pch/pch.h\" /Fp\"${PrecompiledBinary}\""
								OBJECT_OUTPUTS "${PrecompiledBinary}")
	
	set_source_files_properties(${SOURCE_FILES}
								PROPERTIES 
								COMPILE_FLAGS "/Yu\"pch/pch.h\" /FI\"pch/pch.h\" /Fp\"${PrecompiledBinary}\""
							    OBJECT_DEPENDS "${PrecompiledBinary}")  
										   
	list(APPEND SOURCE_FILES pch/pch.cpp)
	list(APPEND SOURCE_FILES platform/windows/resources/WinResource.rc)
endif()

add_executable(Raygen ${SOURCE_FILES} ${HEADER_FILES})
 
if(MSVC)
	set_property(TARGET Raygen PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

# Vulkan
find_package(Vulkan REQUIRED)

# 3rd party libraries
target_link_libraries(Raygen
	XInput.lib
	Vulkan::Vulkan
	glfw
	${GLFW_LIBRARIES}
)

# instruct glfw to globaly include vulkan headers
add_compile_definitions(GLFW_INCLUDE_VULKAN)
add_compile_definitions(GLFW_INCLUDE_NONE)

# dependencies (build order)
add_dependencies(Raygen 
	glfw
)

# 3rd party includes
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY})
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/khr)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/nlohmann)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/ovr)

include_directories(${DEPENDENCIES_DIRECTORY})
include_directories(${DEPENDENCIES_DIRECTORY}/glm)
include_directories(${DEPENDENCIES_DIRECTORY}/spdlog/include)
include_directories(${DEPENDENCIES_DIRECTORY}/imgui)
include_directories(${DEPENDENCIES_DIRECTORY}/magic_enum/include)
include_directories(${DEPENDENCIES_DIRECTORY}/nlohmann/include)
include_directories(${DEPENDENCIES_DIRECTORY}/tinygltf)
include_directories(${DEPENDENCIES_DIRECTORY}/imfilebrowser)
include_directories(${DEPENDENCIES_DIRECTORY}/GLFW/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# for generated exports file
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# enforce cpp17
set_property(TARGET Raygen PROPERTY CXX_STANDARD 17)
set_property(TARGET Raygen PROPERTY CXX_STANDARD_REQUIRED ON)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX ${PROJECT_NAME} FILES ${HEADER_FILES} ${SOURCE_FILES})

#definitions (WINDOWS)
add_compile_definitions(NOMINMAX NODRAWTEXT NOOPENFILE)

# multi-core compilation
if(MSVC)
	target_compile_options(Raygen PRIVATE "/MP")
	# unsafe strcpy used by some dependencies
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
