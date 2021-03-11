#pragma once
// clang-format off

// ALWAYS, ALWAYS keep this before ANY of Raygen's header includes
#include "FeatureMacros.h"

#include "core/Types.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h> // For CommandLineToArgvW

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif


// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
using namespace Microsoft;

// DirectX 12 specific headers.
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <d3dcompiler.h>

// Removing glm is impossible without having to rework most of the engine
// Our primary goal here is to use DirectXMath SIMD and math to accelerate operations
// glm types should mainly be used as storage types with minimal low-cost arithmetic operations
// such as editor math, input math or window/widget sizes, etc...
// World math and rendering math should exclusively use DirectXMath types
#include <DirectXMath.h>

using namespace DirectX;

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

#include "rendering/core/Helpers.h"
#include "rendering/assets/GpuAssetHandle.h"

class World;
#include "universe/Entity.h"


#define IMGUI_DEFINE_MATH_OPERATORS

// clang-format on
