

#ifndef XMODEL_H
#define XMODEL_H

#include "XMesh.h"

#include "assets/DiskAsset.h"
#include "assets/other/xml/XMLDoc.h"

#include <memory>            
#include <string>       

namespace Assets
{
	enum GEOMETRY_TYPE
	{
		GT_DYNAMIC = 0,
		GT_STATIC
	};

	class XModel : public DiskAsset
	{
		GEOMETRY_TYPE m_type;

		std::vector<std::shared_ptr<XMesh>> m_meshes;

		std::shared_ptr<XMLDoc> m_materialsXMLFile;
		std::unordered_map<std::string, std::shared_ptr<XMaterial>> m_materials;

		struct Version
		{
			uint8 major;
			uint8 minor;
			uint8 patch;
		} m_version;

	public:
		XModel(DiskAssetManager* context);
		~XModel() = default;

		bool Load(const std::string& path);
		void Clear() override;

		void SetType(GEOMETRY_TYPE type) { m_type = type; }

		GEOMETRY_TYPE GetType() const { return m_type; }
		std::vector<XMesh*> GetMeshes() const { return Core::GetRawPtrVector(m_meshes); }
		const Version& GetVersion() const { return m_version; }

		std::shared_ptr<XMaterial> GetMaterialByName(const std::string& materialName) const { return m_materials.at(materialName); }

		void ToString(std::ostream& os) const override { os << "asset-type: XModel, name: " << m_label; }
	};
}

#endif // XMODEL_H
