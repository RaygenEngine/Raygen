project (rt-xengine)

#output at dependencies
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DEPENDENCIES_DIRECTORY})

# additional lib dir
link_directories(${LIB_DEPENDENCIES_DIRECTORY})

set(SOURCE_FILES 

	assets/Asset.cpp
	assets/DiskAsset.cpp
	assets/DiskAssetManager.cpp
	assets/PathSystem.cpp
	assets/other/utf8/StringFile.cpp
	assets/other/xml/XMLDoc.cpp
	assets/texture/PackedTexture.cpp
	assets/texture/Texture.cpp
	assets/texture/CubeMap.cpp
	assets/xasset/XMaterial.cpp
	assets/xasset/XMesh.cpp
	assets/xasset/XModel.cpp
	
	core/logger/Logger.cpp
	
	renderer/GPUAsset.cpp
	renderer/Renderer.cpp
	renderer/NodeObserver.cpp
	
	renderer/renderers/opengl/GLAsset.cpp
	renderer/renderers/opengl/GLRendererBase.cpp
	renderer/renderers/opengl/assets/GLInstancedModel.cpp
	renderer/renderers/opengl/assets/GLMaterial.cpp
	renderer/renderers/opengl/assets/GLMesh.cpp
	renderer/renderers/opengl/assets/GLModel.cpp
	renderer/renderers/opengl/assets/GLShader.cpp
	renderer/renderers/opengl/assets/GLTexture.cpp
	renderer/renderers/opengl/assets/GLCubeMap.cpp
	
	renderer/renderers/opengl/test/GLTestRenderer.cpp
	renderer/renderers/opengl/test/GLTestGeometry.cpp

	renderer/renderers/optix/OptixAsset.cpp
	renderer/renderers/optix/OptixRendererBase.cpp
	renderer/renderers/optix/assets/OptixInstancedModel.cpp
	renderer/renderers/optix/assets/OptixMaterial.cpp
	renderer/renderers/optix/assets/OptixMesh.cpp
	renderer/renderers/optix/assets/OptixModel.cpp
	renderer/renderers/optix/assets/OptixProgram.cpp
	renderer/renderers/optix/assets/OptixTexture.cpp
	renderer/renderers/optix/assets/OptixCubeMap.cpp
	
	renderer/renderers/optix/test/OptixTestRenderer.cpp
	renderer/renderers/optix/test/OptixTestGeometry.cpp
	
	renderer/renderers/optix/vr-pathtracer/OptixVRPathTracerGeometry.cpp
	renderer/renderers/optix/vr-pathtracer/OptixVRPathTracerHead.cpp
	renderer/renderers/optix/vr-pathtracer/OptixVRPathTracerLight.cpp
	renderer/renderers/optix/vr-pathtracer/OptixVRPathTracerRenderer.cpp
	
	system/Engine.cpp
	system/EngineObject.cpp
	system/EventListener.cpp
	
	input/Input.cpp
	input/OculusHMD.cpp
	
	event/Event.cpp
	
	world/World.cpp

	world/nodes/Node.cpp
	world/nodes/TransformNode.cpp
	
	world/nodes/camera/CameraNode.cpp
	
	world/nodes/sky/SkyCubeNode.cpp
	world/nodes/sky/SkyHDRNode.cpp
	
	world/nodes/geometry/TriangleModelGeometryNode.cpp
	world/nodes/geometry/TriangleModelInstancedGeometryNode.cpp
	
	world/nodes/light/LightNode.cpp
	
	world/nodes/user/freeform/FreeformUserNode.cpp
	world/nodes/user/oculus/OculusHeadNode.cpp
	world/nodes/user/oculus/OculusUserNode.cpp
	world/nodes/user/UserNode.cpp
	
	main.cpp

	platform/windows/Win32App.cpp
	platform/windows/Win32Window.cpp
)

set(HEADER_FILES 
	assets/Asset.h
	assets/DiskAsset.h
	assets/DiskAssetManager.h
	assets/DiskAssetPart.h
	assets/MultiKeyAssetCacheHashing.h
	assets/PathSystem.h
	assets/other/utf8/StringFile.h
	assets/other/xml/ParsingAux.h
	assets/other/xml/XMLDoc.h
	assets/texture/PackedTexture.h
	assets/texture/Texture.h
	assets/texture/CubeMap.h
	assets/xasset/XMaterial.h
	assets/xasset/XMesh.h
	assets/xasset/XModel.h

	core/auxiliary/FileAux.h
	core/auxiliary/GraphicsMathAux.h
	core/auxiliary/SmartPtrAux.h
	core/auxiliary/StringAux.h
	core/data/BasicLight.h
	core/data/Vertex.h
	core/data/Eye.h
	core/logger/Logger.h
	core/timer/Timer.h
	core/uuid/UUIDGenerator.h
	
	include/rt-xengine.h
	include/common-structs.h

	renderer/GPUAsset.h
	renderer/Renderer.h
	renderer/NodeObserver.h
	
	renderer/renderers/opengl/GLAsset.h
	renderer/renderers/opengl/GLRendererBase.h
	renderer/renderers/opengl/GLUtil.h
	renderer/renderers/opengl/assets/GLInstancedModel.h
	renderer/renderers/opengl/assets/GLMaterial.h
	renderer/renderers/opengl/assets/GLMesh.h
	renderer/renderers/opengl/assets/GLModel.h
	renderer/renderers/opengl/assets/GLShader.h
	renderer/renderers/opengl/assets/GLTexture.h
	renderer/renderers/opengl/assets/GLCubeMap.h

	renderer/renderers/opengl/test/GLTestRenderer.h
	renderer/renderers/opengl/test/GLTestGeometry.h
	
	renderer/renderers/optix/OptixAsset.h
	renderer/renderers/optix/OptixRendererBase.h
	renderer/renderers/optix/OptixUtil.h
	renderer/renderers/optix/assets/OptixInstancedModel.h
	renderer/renderers/optix/assets/OptixMaterial.h
	renderer/renderers/optix/assets/OptixMesh.h
	renderer/renderers/optix/assets/OptixModel.h
	renderer/renderers/optix/assets/OptixProgram.h
	renderer/renderers/optix/assets/OptixTexture.h
	renderer/renderers/optix/assets/OptixCubeMap.h
	
	renderer/renderers/optix/test/OptixTestRenderer.h
	renderer/renderers/optix/test/OptixTestGeometry.h
	
	renderer/renderers/optix/vr-pathtracer/OptixVRPathTracerGeometry.h
	renderer/renderers/optix/vr-pathtracer/OptixVRPathTracerHead.h
	renderer/renderers/optix/vr-pathtracer/OptixVRPathTracerLight.h
	renderer/renderers/optix/vr-pathtracer/OptixVRPathTracerRenderer.h
	
	system/Engine.h
	system/EngineObject.h
	system/EventListener.h

	input/Input.h
	input/OculusHMD.h
	
	system/shared/Event.h
	system/shared/InputEnums.h
	system/shared/RenderTarget.h
	system/shared/TargetEnums.h
	system/shared/Types.h

	event/Event.h

	world/World.h
	
	world/nodes/Node.h
	world/nodes/TransformNode.h
	world/nodes/MetaNodeTranslation.h
	
	world/nodes/camera/CameraNode.h
	
	world/nodes/sky/SkyCubeNode.h
	world/nodes/sky/SkyHDRNode.h
	
	world/nodes/geometry/Instancing.h
	world/nodes/geometry/TriangleModelGeometryNode.h
	world/nodes/geometry/TriangleModelInstancedGeometryNode.h
	
	world/nodes/light/LightNode.h
	
	world/nodes/user/freeform/FreeformUserNode.h
	world/nodes/user/oculus/OculusHeadNode.h
	world/nodes/user/oculus/OculusUserNode.h
	world/nodes/user/UserNode.h

	pch/pch.h
	
	platform/windows/Win32App.h
	platform/windows/Win32Window.h
	platform/windows/TranslateWin32VirtualKeys.h
)

# add pch for msvc 
if(MSVC)

	set(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>_pch/pch.pch")

	set_source_files_properties(pch/pch.cpp
								PROPERTIES 
								COMPILE_FLAGS "/Yc\"pch.h\" /Fp\"${PrecompiledBinary}\""
								OBJECT_OUTPUTS "${PrecompiledBinary}")
	
	set_source_files_properties(${SOURCE_FILES}
								PROPERTIES 
								COMPILE_FLAGS "/Yu\"pch.h\" /FI\"pch.h\" /Fp\"${PrecompiledBinary}\""
							    OBJECT_DEPENDS "${PrecompiledBinary}")  
										   
	list(APPEND SOURCE_FILES pch/pch.cpp)
endif(MSVC)

add_executable(rt-xengine ${SOURCE_FILES} ${HEADER_FILES})


# 3rd party libraries
target_link_libraries(rt-xengine
	optix.6.0.0.lib
	optixu.6.0.0.lib
	GLAD.lib
	tinyxml2.lib
	opengl32.lib
	XInput.lib
)

# glad dll
add_compile_definitions(GLAD_GLAPI_EXPORT)

# dependencies (build order)
add_dependencies(rt-xengine 
	cuda-ptx
	copy-ptx
	glad-dll-gen
)

# 3rd party includes
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/CUDA)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/pch)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# for generated exports file
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# enforce cpp17
set_property(TARGET rt-xengine PROPERTY CXX_STANDARD 17)
set_property(TARGET rt-xengine PROPERTY CXX_STANDARD_REQUIRED ON)

# multi-core compilation
if(MSVC)
	target_compile_options(rt-xengine PRIVATE "/MP")
endif()

# post build copy dlls
add_custom_command(TARGET rt-xengine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${BIN_DEPENDENCIES_DIRECTORY}/$<CONFIGURATION>"
        $<TARGET_FILE_DIR:rt-xengine>
	COMMENT "Copying binary dependencies to target folder...")
