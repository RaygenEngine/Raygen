cmake_minimum_required(VERSION 3.11)

project (Rayxen)

set(PROJECT_MAIN_DIRECTORY "${CMAKE_SOURCE_DIR}")
set(OUTPUT_BIN_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(DEPENDENCIES_DIRECTORY "${CMAKE_SOURCE_DIR}/dependencies")
set(LIB_DEPENDENCIES_DIRECTORY "${DEPENDENCIES_DIRECTORY}/lib")
set(BIN_DEPENDENCIES_DIRECTORY "${DEPENDENCIES_DIRECTORY}/bin")
set(INCLUDE_DEPENDENCIES_DIRECTORY "${DEPENDENCIES_DIRECTORY}/include")

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION "${CMAKE_BINARY_DIR}/external")

macro(AddHeaderOnlyGitDependency name url tag)

	set(INSTALLATION_LOCATION "${EXTERNAL_INSTALL_LOCATION}/${name}")

	message("${INSTALLATION_LOCATION}")

	ExternalProject_Add(${name}
		GIT_REPOSITORY ${url}
		GIT_TAG ${tag}
		PREFIX ${INSTALLATION_LOCATION}
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
		LOG_DOWNLOAD ON
	)

	list(APPEND includes_list "${INSTALLATION_LOCATION}/src")
	list(APPEND includes_list "${INSTALLATION_LOCATION}/src/${name}")
	list(APPEND includes_list "${INSTALLATION_LOCATION}/src/${name}/include")
	list(APPEND header_only_dependencies "${name}")

endmacro()

# occasionally check for tags
AddHeaderOnlyGitDependency("stb" "https://github.com/nothings/stb" "origin/master")
AddHeaderOnlyGitDependency("imfilebrowser" "https://github.com/AirGuanZ/imgui-filebrowser" "origin/master")
AddHeaderOnlyGitDependency("tinygltf" "https://github.com/syoyo/tinygltf" "origin/master")

AddHeaderOnlyGitDependency("glm" "https://github.com/g-truc/glm" "0.9.9.6")
AddHeaderOnlyGitDependency("spdlog" "https://github.com/gabime/spdlog" "v1.4.2")
AddHeaderOnlyGitDependency("imgui" "https://github.com/ocornut/imgui" "v1.73")
AddHeaderOnlyGitDependency("nlohmann" "https://github.com/nlohmann/json" "v3.7.0")
AddHeaderOnlyGitDependency("magic_enum" "https://github.com/Neargye/magic_enum" "v0.6.3")

add_subdirectory (dependencies) 
add_subdirectory (source) 
add_subdirectory (extra-tools/glad-dll-gen)

