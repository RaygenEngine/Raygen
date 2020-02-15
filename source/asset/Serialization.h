#pragma once

#include "asset/AssetPod.h"
#include "asset/AssetManager.h"
#include <fstream>

// Serializes pod -> file
void SerializePodToBinary(PodMetaData& metadata, AssetPod* pod, const fs::path& file);

void DeserializePodFromBinary(PodEntry* entry);
