#pragma once

#include "assets/Asset.h"
#include "assets/model/Mesh.h"

// glTF-based model (not all extensions included) 
class Model : public Asset
{
	GeometryUsage m_usage;
	
	struct Info
	{
		std::string version;
		std::string generator;
		std::string minVersion;
		std::string copyright;
	} m_info;
	
	std::vector<std::unique_ptr<Mesh>> m_meshes;
	
public:
	
	Model(AssetManager* assetManager, const std::string& path)
		: Asset(assetManager, path) {}
	
	bool Load(const std::string& path, GeometryUsage usage);
	void Clear() override;

	const Info& GetInfo() const { return m_info; }

	const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const { return m_meshes; }
	
	GeometryUsage GetUsage() const { return m_usage; }
};

