project(cuda-ptx CUDA)

# find a way to copy only the appropriate ptx files from cuda-ptx
set(SOURCE_FILES 
	test.cu 
	pinhole.cu 
	optixGeometryTriangles.cu
	VRPathTracer.cu
)

set(HEADER_FILES
	test.h 
	commonStructs.h
	helpers.h
	tutorial.h
	VRPathTracerAux.h
	random.h
	lambert_brdf.h
	ggx_brdf.h
)

add_library(cuda-ptx OBJECT ${SOURCE_FILES} ${HEADER_FILES})

set_property(TARGET cuda-ptx PROPERTY CUDA_PTX_COMPILATION ON)


include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/CUDA)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/Optix)
include_directories(${CMAKE_SOURCE_DIR}/rt-xengine)

# Set custom compiler flags
set(CUDA_FAST_MATH TRUE CACHE BOOL "Use Fast Math Operations")

if(CUDA_FAST_MATH)
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -use_fast_math")
endif()

# get a ptx for each cu
foreach(cu_file ${SOURCE_FILES})

	# get basename of cuda source file
	get_filename_component(cu_file_basename ${cu_file} NAME_WE)
	
		list(APPEND COPY_COMMAND  ${CMAKE_COMMAND} -E copy)
		list(APPEND COPY_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dir/$<CONFIGURATION>/${cu_file_basename}.ptx")
		list(APPEND COPY_COMMAND "${CMAKE_SOURCE_DIR}/assets/shaders/ptx")
		list(APPEND COPY_COMMAND "\n")

endforeach()

# copy generated ptx files when ready
add_custom_target(copy-ptx ALL
    COMMAND ${COPY_COMMAND}
	DEPENDS cuda-ptx
	COMMENT "Copying ptx files to assets/shaders/ptx..."
)

