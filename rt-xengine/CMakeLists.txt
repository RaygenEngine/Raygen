project (rt-xengine)

#output at dependencies
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DEPENDENCIES_DIRECTORY})

# additional lib dir
link_directories(${LIB_DEPENDENCIES_DIRECTORY})

set(SOURCE_FILES 

	assets/Asset.cpp
	assets/AssetManager.cpp
	assets/PathSystem.cpp
	
	assets/other/utf8/StringFile.cpp
	assets/other/xml/XMLDoc.cpp
	
	assets/texture/PackedTexture.cpp
	assets/texture/Texture.cpp
	assets/texture/CubeMap.cpp
	
	assets/model/Model.cpp
	
	core/logger/Logger.cpp
	
	renderer/Renderer.cpp
	renderer/NodeObserver.cpp
	
	renderer/renderers/opengl/GLAsset.cpp
	renderer/renderers/opengl/GLAssetManager.cpp
	renderer/renderers/opengl/GLRendererBase.cpp
	renderer/renderers/opengl/assets/GLInstancedModel.cpp
	renderer/renderers/opengl/assets/GLModel.cpp
	renderer/renderers/opengl/assets/GLShader.cpp
	renderer/renderers/opengl/assets/GLTexture.cpp
	renderer/renderers/opengl/assets/GLCubeMap.cpp
	
	renderer/renderers/opengl/test/GLTestRenderer.cpp
	renderer/renderers/opengl/test/GLTestGeometry.cpp

	system/Engine.cpp
	system/EngineComponent.cpp
	system/Object.cpp
	system/Input.cpp
	
	event/Event.cpp
	
	world/World.cpp
	world/NodeFactory.cpp
	
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
	AppBase.cpp

	platform/Window.cpp
	platform/windows/Win32Window.cpp
)

set(HEADER_FILES 

	assets/Asset.h
	assets/AssetManager.h
	assets/PathSystem.h
    assets/CachingAux.h
		
	assets/other/utf8/StringFile.h
	assets/other/xml/ParsingAux.h
	assets/other/xml/XMLDoc.h
	
	assets/texture/PackedTexture.h
	assets/texture/Texture.h
	assets/texture/CubeMap.h
	
	assets/model/Model.h
	assets/model/GltfAux.h

	core/auxiliary/FileAux.h
	core/auxiliary/GraphicsMathAux.h
	core/auxiliary/SmartPtrAux.h
	core/auxiliary/StringAux.h

	core/data/BasicLight.h
	core/data/Vertex.h
	
	core/enum/InputEnums.h
	core/enum/TargetEnums.h
	core/enum/TextureEnums.h
	core/enum/GeometryEnums.h
	core/enum/BufferEnums.h
	
	core/logger/Logger.h
	
	core/timer/Timer.h
	
	core/type/IntegerTypes.h
	core/type/MathTypes.h
	
	core/uuid/UUIDGenerator.h
	
	renderer/Renderer.h
	renderer/NodeObserver.h
	
	renderer/renderers/opengl/GLAsset.h
	renderer/renderers/opengl/GLAssetManager.h
	renderer/renderers/opengl/GLRendererBase.h
	renderer/renderers/opengl/GLUtil.h
	renderer/renderers/opengl/assets/GLInstancedModel.h
	renderer/renderers/opengl/assets/GLModel.h
	renderer/renderers/opengl/assets/GLShader.h
	renderer/renderers/opengl/assets/GLTexture.h
	renderer/renderers/opengl/assets/GLCubeMap.h

	renderer/renderers/opengl/test/GLTestRenderer.h
	renderer/renderers/opengl/test/GLTestGeometry.h
	
	system/Engine.h
	system/EngineComponent.h
	system/Object.h
	system/Input.h
	
	event/Event.h

	world/World.h
	world/NodeFactory.h
	
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
	
	AppBase.h
	
	platform/Window.h
	platform/windows/Win32Window.h
	platform/windows/TranslateWin32VirtualKeys.h
)


if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app/CMakeLists.txt" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app/App.h")
	set(APP_DIR "app")
	message(STATUS "Detected app header at 'rt-xengine/app'. Using custom app files.") 
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
	glad.lib
	tinyxml2.lib
	opengl32.lib
	XInput.lib
)

# glad dll
add_compile_definitions(GLAD_GLAPI_EXPORT)

# dependencies (build order)
add_dependencies(rt-xengine 
	glad-dll-gen
)

# 3rd party includes
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY})
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/GLAD)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/GLM)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/KHR)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/nlohmann)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/spdlog)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/stb_image)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/tinygltf)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/tinyxml2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/pch)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# for generated exports file
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# enforce cpp17
set_property(TARGET rt-xengine PROPERTY CXX_STANDARD 17)
set_property(TARGET rt-xengine PROPERTY CXX_STANDARD_REQUIRED ON)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX ${PROJECT_NAME} FILES ${HEADER_FILES} ${SOURCE_FILES})

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
