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

#include "core/BoolFlag.h"
#include "core/Icons.h"
#include "core/MacroUtl.h"
#include "core/MathUtl.h"
#include "core/MetaTemplates.h"
#include "core/StringUtl.h"
#include "core/Hash.h"

#include "engine/Logger.h"

#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.hpp>

#define MAGIC_ENUM_RANGE_MIN -1
#define MAGIC_ENUM_RANGE_MAX 24
#include <magic_enum.hpp>

#include <entt/src/entt/entity/registry.hpp>

namespace fs = std::filesystem;
namespace ch = std::chrono;
// Engine stuff

#include "assets/PodHandle.h"
#include "reflection/GenMacros.h"

#include "rendering/assets/GpuAssetHandle.h"

constexpr size_t c_framesInFlight = 2;
template<typename T>
struct InFlightResource : std::array<T, c_framesInFlight> {
	// NEXT:
	InFlightResource(std::vector<T>&& data) { std::move(data.begin(), data.begin() + c_framesInFlight, begin()); }
	InFlightResource(T val) { for (auto& v : *this) {v = val} }
	InFlightResource() = default;



	void operator=(const std::vector<T>& data) { std::move(data.begin(), data.begin() + c_framesInFlight, begin()); }
};

#include "rendering/core/img.h"
