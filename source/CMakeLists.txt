project (Raygen-App)
project (Raygen)


file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "*.cpp")
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/AppMain.cpp")
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp")
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
										   
	list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp")
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
	${RAYGEN_SPIRV_CROSS_MODULES}

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
target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY}/spirv-cross/include)


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
