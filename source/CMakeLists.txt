project (Rayxen)

#output at dependencies
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DEPENDENCIES_DIRECTORY})

# additional lib dir
link_directories(${LIB_DEPENDENCIES_DIRECTORY})

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
	
	platform/windows/Win32Window.cpp
	

	renderer/ObserverRenderer.cpp
	

	renderer/renderers/opengl/GLEditorRenderer.cpp
	renderer/renderers/opengl/GLRendererBase.cpp
	renderer/renderers/opengl/assets/GLModel.cpp
	renderer/renderers/opengl/assets/GLShader.cpp
	renderer/renderers/opengl/assets/GLTexture.cpp
	renderer/renderers/opengl/assets/GLMaterial.cpp
	renderer/renderers/opengl/basic/GLBasicSpotLight.cpp
	renderer/renderers/opengl/basic/GLBasicDirectionalLight.cpp
	renderer/renderers/opengl/basic/GLBasicPunctualLight.cpp
	renderer/renderers/opengl/basic/GLBasicGeometry.cpp
	renderer/renderers/opengl/forward/GLForwardRenderer.cpp
	renderer/renderers/opengl/deferred/GLDeferredRenderer.cpp
	renderer/renderers/opengl/previewer/GLPreviewerRenderer.cpp


	system/Engine.cpp
	system/Input.cpp
	system/Logger.cpp


	world/World.cpp
	world/NodeFactory.cpp
	
	world/nodes/Node.cpp
	world/nodes/RootNode.cpp
	world/nodes/camera/CameraNode.cpp
	world/nodes/camera/WindowCameraNode.cpp
	world/nodes/camera/EditorCameraNode.cpp
	world/nodes/geometry/GeometryNode.cpp
	world/nodes/light/PunctualLightNode.cpp
	world/nodes/light/DirectionalLightNode.cpp
	world/nodes/light/SpotLightNode.cpp
	world/nodes/light/LightNode.cpp
	world/nodes/user/FreeformUserNode.cpp
	world/nodes/user/UserNode.cpp
)

set(HEADER_FILES
	AppBase.h

 
	asset/AssetPod.h
	asset/AssetManager.h
	asset/UriLibrary.h
	asset/PodIncludes.h
	asset/PodHandle.h

	asset/loaders/CubemapLoader.h
	asset/loaders/DummyLoader.h
	asset/loaders/TextLoader.h
	asset/loaders/JsonDocLoader.h
	asset/loaders/ImageLoader.h
	asset/loaders/ShaderLoader.h
	asset/loaders/GltfModelLoader.h
	asset/loaders/GltfMaterialLoader.h
	asset/loaders/GltfTextureLoader.h	
	asset/loaders/GltfFileLoader.h

	asset/pods/TexturePod.h
	asset/pods/MaterialPod.h
	asset/pods/ModelPod.h
	asset/pods/ImagePod.h
	asset/pods/ShaderPod.h
	asset/pods/StringPod.h
	asset/pods/JsonDocPod.h
	asset/pods/GltfFilePod.h

	asset/util/GltfAux.h
	asset/util/ParsingAux.h


	core/CoreEnums.h
	core/CoreStructs.h
	core/Event.h
	core/MathAux.h
	core/MetaTemplates.h
	core/StringAux.h
	core/Types.h


	editor/AssetWindow.h
	editor/Editor.h
	editor/NodeContextActions.h
	editor/PropertyEditor.h
	editor/SceneSave.h

	editor/imgui/ImguiImpl.h
	editor/imgui/ImguiUtil.h

	
	pch/pch.h
	
	
	platform/Window.h
	
	platform/windows/Win32Window.h
	platform/windows/TranslateWin32VirtualKeys.h
	

	reflection/GenMacros.h
	reflection/GetClass.h
	reflection/PodReflection.h
	reflection/PodTools.h
	reflection/Property.h
	reflection/PropertyFlags.h
	reflection/PropertyTypes.h
	reflection/ReflClass.h
	reflection/ReflectionTools.h
	reflection/ReflEnum.h
	reflection/TypeId.h

	renderer/Renderer.h
	renderer/ObserverRenderer.h
	renderer/NodeObserver.h
	renderer/GenericGpuAssetManager.h
	
	renderer/util/GLSLutil.h
	
	renderer/renderers/opengl/GLAsset.h
	renderer/renderers/opengl/GLAssetManager.h
	renderer/renderers/opengl/GLEditorRenderer.h
	renderer/renderers/opengl/GLRendererBase.h
	renderer/renderers/opengl/GLUtil.h
	renderer/renderers/opengl/assets/GLModel.h
	renderer/renderers/opengl/assets/GLShader.h
	renderer/renderers/opengl/assets/GLTexture.h
	renderer/renderers/opengl/assets/GLMaterial.h
	renderer/renderers/opengl/basic/GLBasicSpotLight.h
	renderer/renderers/opengl/basic/GLBasicDirectionalLight.h
	renderer/renderers/opengl/basic/GLBasicPunctualLight.h
	renderer/renderers/opengl/basic/GLBasicGeometry.h
	renderer/renderers/opengl/forward/GLForwardRenderer.h
	renderer/renderers/opengl/deferred/GLDeferredRenderer.h
	renderer/renderers/opengl/previewer/GLPreviewerRenderer.h


	system/Engine.h
	system/Object.h
	system/Input.h
	system/InputEnums.h
	system/EngineEvents.h
	system/Logger.h
	system/Timer.h
	system/UUIDGenerator.h


	world/World.h
	world/NodeFactory.h
	
	world/nodes/Node.h
	world/nodes/RootNode.h
	world/nodes/TransformNode.h
	world/nodes/camera/CameraNode.h
	world/nodes/camera/WindowCameraNode.h
	world/nodes/camera/EditorCameraNode.h
	world/nodes/sky/SkyboxNode.h
	world/nodes/geometry/GeometryNode.h
	world/nodes/light/PunctualLightNode.h
	world/nodes/light/DirectionalLightNode.h
	world/nodes/light/SpotLightNode.h
	world/nodes/light/LightNode.h
	world/nodes/user/FreeformUserNode.h
	world/nodes/user/UserNode.h
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
								COMPILE_FLAGS "/Yc\"pch/pch.h\" /Fp\"${PrecompiledBinary}\""
								OBJECT_OUTPUTS "${PrecompiledBinary}")
	
	set_source_files_properties(${SOURCE_FILES}
								PROPERTIES 
								COMPILE_FLAGS "/Yu\"pch/pch.h\" /FI\"pch/pch.h\" /Fp\"${PrecompiledBinary}\""
							    OBJECT_DEPENDS "${PrecompiledBinary}")  
										   
	list(APPEND SOURCE_FILES pch/pch.cpp)
endif(MSVC)

add_executable(Rayxen ${SOURCE_FILES} ${HEADER_FILES})
 
if(MSVC)
	set_property(TARGET Rayxen PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif(MSVC)

# 3rd party libraries
target_link_libraries(Rayxen
	glad.lib
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
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/glad)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/glm)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/khr)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/nlohmann)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/spdlog)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/stb_image)
include_directories(${INCLUDE_DEPENDENCIES_DIRECTORY}/tinygltf)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# for generated exports file
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# enforce cpp17
set_property(TARGET Rayxen PROPERTY CXX_STANDARD 17)
set_property(TARGET Rayxen PROPERTY CXX_STANDARD_REQUIRED ON)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX ${PROJECT_NAME} FILES ${HEADER_FILES} ${SOURCE_FILES})

#definitions (WINDOWS)
add_compile_definitions(NOMINMAX NODRAWTEXT NOOPENFILE)

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
