project (Rayxen)

#output at dependencies
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DEPENDENCIES_DIRECTORY})

# additional lib dir
link_directories(${LIB_DEPENDENCIES_DIRECTORY})

set(SOURCE_FILES 

	asset/Asset.cpp
	asset/AssetManager.cpp
	asset/PathSystem.cpp
	asset/PodLoaders.cpp
	
	asset/assets/TextAsset.cpp
	asset/assets/XMLDocAsset.cpp
	asset/assets/ImageAsset.cpp
	asset/assets/CubemapAsset.cpp
	asset/assets/ShaderAsset.cpp
	asset/assets/GltfModelAsset.cpp
	asset/assets/GltfMaterialAsset.cpp	
	asset/assets/GltfTextureAsset.cpp
	asset/assets/GltfFileAsset.cpp
	asset/assets/DummyAssets.cpp
	
	asset/util/DependenciesImpl.cpp

	core/logger/Logger.cpp
	
	editor/Editor.cpp
	editor/imgui/ImguiImpl.cpp
	editor/imgui/ImguiDemo.cpp
	editor/renderer/EditorRenderer.cpp

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
	renderer/renderers/opengl/assets/GLMaterial.cpp
	
	renderer/renderers/opengl/test/GLTestRenderer.cpp
	renderer/renderers/opengl/test/GLTestGeometry.cpp

	system/Engine.cpp
	system/EngineComponent.cpp
	system/Object.cpp
	system/Input.cpp
	
	system/reflection/Reflection.cpp
	
	event/Event.cpp
	
	world/World.cpp
	world/NodeFactory.cpp
	
	world/nodes/Node.cpp
	world/nodes/TransformNode.cpp
	
	world/nodes/camera/CameraNode.cpp
	
	world/nodes/sky/SkyCubeNode.cpp
	world/nodes/sky/SkyHDRNode.cpp
	
	world/nodes/geometry/GeometryNode.cpp
	world/nodes/geometry/InstancedGeometryNode.cpp
	
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

	asset/Asset.h
	asset/AssetPod.h
	asset/AssetManager.h
	asset/PathSystem.h
		
	asset/assets/TextAsset.h
	asset/assets/XMLDocAsset.h
	asset/assets/ImageAsset.h
	asset/assets/CubemapAsset.h
	asset/assets/ShaderAsset.h
	asset/assets/GltfModelAsset.h
	asset/assets/GltfMaterialAsset.h
	asset/assets/GltfTextureAsset.h	
	asset/assets/GltfFileAsset.h
	asset/assets/DummyAssets.h

	asset/pods/TexturePod.h
	asset/pods/MaterialPod.h
	asset/pods/ModelPod.h
	asset/pods/ImagePod.h
	asset/pods/ShaderPod.h
	asset/pods/TextPod.h
	asset/pods/CubemapPod.h
	asset/pods/XMLDocPod.h
	asset/pods/GltfFilePod.h
	
	asset/util/GltfAux.h
	asset/util/ParsingAux.h

	core/auxiliary/FlagAux.h
	core/auxiliary/FileAux.h
	core/auxiliary/GraphicsMathAux.h
	core/auxiliary/MetaTemplates.h
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
	
	editor/Editor.h
	editor/imgui/ImguiImpl.h
	editor/imgui/ImguiExtensions.h
	editor/renderer/EditorRenderer.h
	
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
	renderer/renderers/opengl/assets/GLMaterial.h

	renderer/renderers/opengl/test/GLTestRenderer.h
	renderer/renderers/opengl/test/GLTestGeometry.h
	
	system/Engine.h
	system/EngineComponent.h
	system/Object.h
	system/Input.h
	system/EngineEvents.h

	system/reflection/Reflection.h
	system/reflection/Reflector.h
	system/reflection/Property.h
	
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
	world/nodes/geometry/GeometryNode.h
	world/nodes/geometry/InstancedGeometryNode.h
	
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
								COMPILE_FLAGS "/Yc\"pch.h\" /Fp\"${PrecompiledBinary}\""
								OBJECT_OUTPUTS "${PrecompiledBinary}")
	
	set_source_files_properties(${SOURCE_FILES}
								PROPERTIES 
								COMPILE_FLAGS "/Yu\"pch.h\" /FI\"pch.h\" /Fp\"${PrecompiledBinary}\""
							    OBJECT_DEPENDS "${PrecompiledBinary}")  
										   
	list(APPEND SOURCE_FILES pch/pch.cpp)
endif(MSVC)

add_executable(Rayxen ${SOURCE_FILES} ${HEADER_FILES})


# 3rd party libraries
target_link_libraries(Rayxen
	glad.lib
	tinyxml2.lib
	opengl32.lib
	XInput.lib
)

# glad dll
add_compile_definitions(GLAD_GLAPI_EXPORT)

# dependencies (build order)
add_dependencies(Rayxen 
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
set_property(TARGET Rayxen PROPERTY CXX_STANDARD 17)
set_property(TARGET Rayxen PROPERTY CXX_STANDARD_REQUIRED ON)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX ${PROJECT_NAME} FILES ${HEADER_FILES} ${SOURCE_FILES})

# multi-core compilation
if(MSVC)
	target_compile_options(Rayxen PRIVATE "/MP")
endif()

# post build copy dlls
add_custom_command(TARGET Rayxen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${BIN_DEPENDENCIES_DIRECTORY}/$<CONFIGURATION>"
        $<TARGET_FILE_DIR:Rayxen>
	COMMENT "Copying binary dependencies to target folder...")
