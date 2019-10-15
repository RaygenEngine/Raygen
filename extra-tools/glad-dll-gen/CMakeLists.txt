project(glad-dll-gen)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DEPENDENCIES_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DEPENDENCIES_DIRECTORY})

set(SOURCE_FILES 
	glad.c
)

set(HEADER_FILES 
	glad/glad.h
	khr/khrplatform.h
)

add_library(glad-dll-gen SHARED ${SOURCE_FILES} ${HEADER_FILES})


set_target_properties(glad-dll-gen PROPERTIES OUTPUT_NAME "glad")

add_compile_definitions(GLAD_GLAPI_EXPORT)
add_compile_definitions(GLAD_GLAPI_EXPORT_BUILD)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# multi-core compilation
if(MSVC)
	target_compile_options(glad-dll-gen PRIVATE "/MP")
endif()

# post build create include dirs
add_custom_command(TARGET glad-dll-gen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory 
		"${INCLUDE_DEPENDENCIES_DIRECTORY}/glad"
	COMMAND ${CMAKE_COMMAND} -E make_directory 
		"${INCLUDE_DEPENDENCIES_DIRECTORY}/khr"
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	    "${CMAKE_CURRENT_SOURCE_DIR}/glad"
        "${INCLUDE_DEPENDENCIES_DIRECTORY}/glad"
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	    "${CMAKE_CURRENT_SOURCE_DIR}/khr"
        "${INCLUDE_DEPENDENCIES_DIRECTORY}/khr"
	COMMENT "Copying glad include directories..."
)