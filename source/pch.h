#pragma once
// clang-format off

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

// Should be in some header probably
struct TextCompilerErrors {
	std::map<int, std::string> errors;
	bool wasSuccessful{ false };
};


class World;
#include "universe/Entity.h"


#define IMGUI_DEFINE_MATH_OPERATORS

// clang-format on
