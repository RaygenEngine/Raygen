#pragma once

#include "assets/Asset.h"

namespace tinygltf
{
	class Model;
}

class GltfFile : public Asset
{
	tinygltf::Model* m_gltfData;

public:
	GltfFile(const fs::path& path)
		: Asset(path),
	      m_gltfData(nullptr) {}
	~GltfFile() = default;

	tinygltf::Model* GetGltfData() const { return m_gltfData; }

protected:
	bool Load() override;
	void Unload() override;
};

