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

// int types
#include "core/Types.h"
// enums/structs
#include "core/CoreEnums.h"
#include "core/CoreStructs.h"
// meta
#include "core/MetaTemplates.h"
