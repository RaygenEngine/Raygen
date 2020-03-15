#pragma once
#include "asset/AssetManager.h"
#include "asset/AssetPod.h"

#include <fstream>

// Serializes pod -> file
void SerializePodToBinary(PodMetaData& metadata, AssetPod* pod, const fs::path& file);

void DeserializePodFromBinary(PodEntry* entry);
