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

#include <vulkan/vulkan.hpp>

#define MAGIC_ENUM_RANGE_MIN -1
#define MAGIC_ENUM_RANGE_MAX 24
#include <magic_enum.hpp>

namespace fs = std::filesystem;

// Engine stuff

#include "assets/PodHandle.h"
#include "reflection/GenMacros.h"
