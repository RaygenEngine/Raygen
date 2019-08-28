#pragma once

#include "assets/DiskAsset.h"
#include "assets/model/Mesh.h"

namespace Assets
{
	// glTF-based model (not all extensions included) 
	class Model : public DiskAsset
	{
		GeometryUsage m_usage;
		
		struct Info
		{
			std::string version;
			std::string generator;
			std::string minVersion;
			std::string copyright;
		} m_info;
		
		std::vector<Mesh> m_meshes;
		
	public:
		
		Model(EngineObject* pObject, const std::string& path);
		
		bool Load(const std::string& path, GeometryUsage usage);
		void Clear() override;

		const Info& GetInfo() const { return m_info; }

		const std::vector<Mesh>& GetMeshes() const { return m_meshes; }
		
		GeometryUsage GetUsage() const { return m_usage; }

		void ToString(std::ostream& os) const override { os << "asset-type: Model, name: " << m_name; }
	};
}
