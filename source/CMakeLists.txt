project (Raygen-App)
project (Raygen)

function (filter_items aItems aRegEx)
    # For each item in our list
    foreach (item ${${aItems}})
        # Check if our items matches our regular expression
        if ("${item}" MATCHES ${aRegEx})
            # Remove current item from our list
            list (REMOVE_ITEM ${aItems} ${item})
        endif ("${item}" MATCHES ${aRegEx})
    endforeach(item)
    # Provide output parameter
    set(${aItems} ${${aItems}} PARENT_SCOPE)
endfunction (filter_items)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "*.cpp")
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/AppMain.cpp")
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/empty.cpp")
filter_items(SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dynlib/.*.cpp")
list(REMOVE_ITEM SOURCE_FILES DYNLIB_FILES)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "*.h")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Raygen" FILES ${HEADER_FILES} ${SOURCE_FILES})

set(APP_SOURCES
	AppMain.cpp
)

project (Raygen-Dynlib)
file(GLOB_RECURSE SOURCE_FILES_DYN CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/dynlib/*.cpp")
file(GLOB_RECURSE HEADER_FILES_DYN CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/dynlib/*.h")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Raygen-Dynlib" FILES ${HEADER_FILES_DYN} ${SOURCE_FILES_DYN})
set ( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )
add_library(Raygen-Dynlib SHARED ${SOURCE_FILES_DYN} ${HEADER_FILES_DYN})
# Hacked below to build with optimisations on debug config.
set ( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF )

# Include shaders to a separate project
project (Raygen-Shaders)
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../assets/engine-data/spv/")
file(GLOB_RECURSE SHADER_SOURCES CONFIGURE_DEPENDS "${SHADER_DIR}/*.*")
list(FILTER SHADER_SOURCES EXCLUDE REGEX ".spv$")
source_group(TREE ${SHADER_DIR} PREFIX "Shaders" FILES ${SHADER_SOURCES})
set (DUMMY_CPP "${CMAKE_CURRENT_SOURCE_DIR}/empty.cpp")
add_library(Raygen-Shaders ${SHADER_SOURCES} ${DUMMY_CPP})


# add Icon for windows binary
if(MSVC)
	list(APPEND APP_SOURCES platform/windows/resources/WinResource.rc)
endif()


add_library(Raygen STATIC ${SOURCE_FILES} ${HEADER_FILES})
add_executable(Raygen-App ${APP_SOURCES})

# Must be used after add_library because CMake needs to know that pch.h is used in the particlar project
target_precompile_headers(Raygen PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/pch.h") 
target_precompile_headers(Raygen-Dynlib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")  

if(MSVC)
	# TODO: this does not work anymore?
	set_property(TARGET Raygen-App PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

# 3rd party libraries
target_link_libraries(Raygen
	NativeFileDialog
	XInput.lib
	ImGui
	spdlog
	glfw
	${GLFW_LIBRARIES}
	ImGuiColorTextEdit
    Raygen-Dynlib
)

target_link_libraries(Raygen-App
	Raygen
)

# instruct glfw to globaly include vulkan headers
add_compile_definitions(GLFW_INCLUDE_NONE)

# dependencies (build order)
add_dependencies(Raygen 
	ImGui
	glfw
	spdlog
)

# should figure out a cross platform way to build the dynamic lib on release always. 
if (MSVC) 
	# also remove this stupid rtc removal hack
	#
	# the scope of regex removal seems to be for this folder only so we should not need to re-add it to all the 
	# dependencies but who knows what happens in the future
	STRING (REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")
	target_compile_options(Raygen-Dynlib PRIVATE "/O2" "/Ob2")
	foreach(target_var Raygen-App Raygen)
		target_compile_options(${target_var} PRIVATE $<$<CONFIG:Debug>: /RTC1>)
	endforeach()
endif()

#target_include_directories(Raygen PUBLIC ${Vulkan_INCLUDE_DIR})
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
#target_include_directories(Raygen PUBLIC ${DEPENDENCIES_DIRECTORY}/spirv-cross/include)

target_include_directories(Raygen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Raygen PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# enforce cpp20
set_property(TARGET Raygen Raygen-App Raygen-Dynlib PROPERTY CXX_STANDARD 20)
set_property(TARGET Raygen Raygen-App Raygen-Dynlib PROPERTY CXX_STANDARD_REQUIRED ON)


#definitions (WINDOWS)
add_compile_definitions(NOMINMAX NODRAWTEXT NOOPENFILE) # CHECK:

# multi-core compilation
if(MSVC)
	target_compile_options(Raygen PRIVATE "/MP")
	# target_compile_options(Raygen PRIVATE "/experimental:preprocessor")

	# unsafe strcpy used by some dependencies
	add_definitions(-D_CRT_SECURE_NO_WARNINGS) # CHECK:
	
endif()

# Copy all includes from "Raygen" target to "Raygen-Dynlib"
get_target_property(RAYGEN_INCLUDE_DIRS Raygen INCLUDE_DIRECTORIES)
target_include_directories(Raygen-Dynlib PRIVATE ${RAYGEN_INCLUDE_DIRS})
