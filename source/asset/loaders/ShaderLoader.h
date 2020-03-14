#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/ShaderPod.h"
#include "asset/UriLibrary.h"

// if found same included file then abort
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <fstream>

namespace ShaderLoader {
inline void Load(ShaderPod* pod, const uri::Uri& path) {}


}; // namespace ShaderLoader
