#pragma once

#include "assets/Asset.h"

namespace tinygltf
{
	class Model;
}

class GltfFileAsset : public Asset
{
	tinygltf::Model* m_gltfData;

public:
	GltfFileAsset(const fs::path& path)
		: Asset(path),
	      m_gltfData(nullptr) {}
	~GltfFileAsset() = default;

	tinygltf::Model* GetGltfData() const { return m_gltfData; }

protected:
	bool Load() override;
	void Unload() override;
};

