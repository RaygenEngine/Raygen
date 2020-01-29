project (Raygen-Test)

# 'Bad' practice to glob .cpp files but it removes barrier entry to adding proper tests.
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "*.cpp") 
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "*.h")

add_executable(Raygen-Test ${SOURCE_FILES} ${HEADER_FILES})
 
if(MSVC)
	set_property(TARGET Raygen-Test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	target_compile_options(Raygen-Test PRIVATE "/MP")
endif()

target_link_libraries(Raygen-Test 
	Raygen
)

# Local Includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Additional dependencies needed only by Raygen-Test
include_directories(${DEPENDENCIES_DIRECTORY}/catch2/single_include)

message(WARN "${DEPENDENCIES_DIRECTORY}")

# enforce cpp20
set_property(TARGET Raygen-Test PROPERTY CXX_STANDARD 20)
set_property(TARGET Raygen-Test PROPERTY CXX_STANDARD_REQUIRED ON)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX ${PROJECT_NAME} FILES ${HEADER_FILES} ${SOURCE_FILES})
