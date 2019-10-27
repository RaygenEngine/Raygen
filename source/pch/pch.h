#pragma once

#include <filesystem>
namespace fs = std::filesystem;

#include <cinttypes>
#include <cstddef>


#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CXX17
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <type_traits>

// ALWAYS, ALWAYS keep this before ANY of Rayxen's header includes
#include "FeatureMacros.h"

// int types
#include "core/Types.h"
// enums/structs
#include "core/CoreEnums.h"
// meta
#include "core/MetaTemplates.h"
