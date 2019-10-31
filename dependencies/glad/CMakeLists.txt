project(glad)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(SOURCE_FILES 
	src/glad.c
)

set(HEADER_FILES 
	include/glad/glad.h
	include/KHR/khrplatform.h
)

add_library(glad SHARED ${SOURCE_FILES} ${HEADER_FILES})

set_target_properties(glad PROPERTIES OUTPUT_NAME "glad")

add_compile_definitions(GLAD_GLAPI_EXPORT)
add_compile_definitions(GLAD_GLAPI_EXPORT_BUILD)

include_directories(${DEPENDENCIES_DIRECTORY}/glad/include)

# multi-core compilation
if(MSVC)
	target_compile_options(glad PRIVATE "/MP")
endif()
