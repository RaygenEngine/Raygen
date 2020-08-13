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
struct FrameArray : std::array<T, c_framesInFlight> {
	// CHECK: for non int/bool
	FrameArray(const std::vector<T>&& data) { std::move(data.begin(), data.begin() + c_framesInFlight, begin()); }
	FrameArray(T val) { std::memset(this, val, sizeof(T) * c_framesInFlight); }
	FrameArray() = default;

	void operator=(const std::vector<T>& data) { std::move(data.begin(), data.begin() + c_framesInFlight, begin()); }
};
