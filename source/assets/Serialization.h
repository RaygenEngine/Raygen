#pragma once
#include "assets/AssetManager.h"
#include "assets/AssetPod.h"

#include <fstream>

// Serializes pod -> file
void SerializePodToBinary(PodMetaData& metadata, AssetPod* pod, const fs::path& file);

void DeserializePodFromBinary(PodEntry* entry);
