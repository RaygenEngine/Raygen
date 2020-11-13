#pragma once


// ALWAYS, ALWAYS keep this before ANY of Raygen's header includes
#include "FeatureMacros.h"

#include "core/Types.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <functional>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <xtree>
#include <span>

#include "core/BoolFlag.h"
#include "core/Icons.h"
#include "core/MacroUtl.h"
#include "core/MetaTemplates.h"
#include "core/Concepts.h"
#include "core/MathUtl.h"
#include "core/StringUtl.h"
#include "core/Hash.h"
#include "core/MemorySpan.h"

#include "engine/Logger.h"

constexpr size_t c_framesInFlight = 2;

template<typename T>
struct InFlightResources : std::array<T, c_framesInFlight> {
	InFlightResources(std::vector<T>&& data)
	{
		std::move(data.begin(), data.begin() + c_framesInFlight, this->begin());
	}
	InFlightResources(T val)
	{
		for (auto& v : *this) {
			v = val;
		}
	}
	InFlightResources() = default;


	void operator=(const std::vector<T>& data)
	{
		std::move(data.begin(), data.begin() + c_framesInFlight, this->begin());
	}
};

#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.hpp>

#define MAGIC_ENUM_RANGE_MIN -1
#define MAGIC_ENUM_RANGE_MAX 24
#include <magic_enum.hpp>

//@ MODULES:
//#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_JSON
#include <stb/stb_image.h>
#include <nlohmann/json.hpp>
#include <tinygltf/tiny_gltf.h>
//#define STB_IMAGE_IMPLEMENTATION


#include <entt/src/entt/entity/registry.hpp>

namespace fs = std::filesystem;
namespace ch = std::chrono;
// Engine stuff

#include "assets/PodHandle.h"
#include "universe/ComponentsDb.h"
#include "reflection/GenMacros.h"

//@ MODULES:

#include "rendering/DebugName.h"

#include "assets/PodIncludes.h"
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuCubemap.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuMaterialArchetype.h"
#include "rendering/assets/GpuMaterialInstance.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuSampler.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/assets/GpuSkinnedMesh.h"


#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneDirlight.h"
#include "rendering/scene/SceneIrradianceGrid.h"
#include "rendering/scene/ScenePointlight.h"
#include "rendering/scene/SceneReflprobe.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/scene/SceneSpotlight.h"


#include "rendering/DebugName.h"
#include "rendering/core/img.h"

// class World;
#include "universe/Entity.h"


#define IMGUI_DEFINE_MATH_OPERATORS
