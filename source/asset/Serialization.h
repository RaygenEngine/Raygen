#pragma once

#include "asset/AssetPod.h"
#include <fstream>

// Serializes pod -> file
void SerializePod(AssetPod* pod, const fs::path& file);

// Desirializes ONTO pod from file, can fail if types mismatch
void DeserializePod(AssetPod* pod, const fs::path& file);
