#include "pch.h"
#include "XModel.h"

#include "assets/other/xml/ParsingAux.h"


namespace Assets
{
	XModel::XModel(DiskAssetManager* context)
		: DiskAsset(context),
		  m_type(),
		  m_materialsXMLFile(nullptr),
		  m_version()
	{
	}

	bool XModel::Load(const std::string& path)
	{
		SetIdentificationFromPath(path);

		// SetIdentificationFromAssociatedDiskAssetIdentification from file

		std::ifstream xmdFile(m_path, std::ifstream::ate | std::ios::binary);
		if (!xmdFile.is_open())
		{
			RT_XENGINE_LOG_WARN("Could not open xmd file, path: {}", m_path);
			return false;
		}

		const auto size = xmdFile.tellg();
		xmdFile.seekg(0, std::ios::beg);

		Core::XMDFileData data;
		data.buffer.resize(size);

		Core::ReadBufferLittleEndianFromFile(xmdFile, data.buffer.data(), static_cast<uint32>(size));

		// SetIdentificationFromAssociatedDiskAssetIdentification from xmd data

		RT_XENGINE_LOG_INFO("Loading XModel, name: {}", PathSystem::GetNameWithExtension(m_path));

		ReadValueLittleEndianFromBuffer(data, m_version.major);
		ReadValueLittleEndianFromBuffer(data, m_version.minor);
		ReadValueLittleEndianFromBuffer(data, m_version.patch);

		RT_XENGINE_LOG_TRACE("XMD Version found: {0}.{1}.{2}", m_version.major, m_version.minor, m_version.patch);

		uint32 meshCount;
		ReadValueLittleEndianFromBuffer(data, meshCount);
		RT_XENGINE_LOG_TRACE("Mesh count: {0}", meshCount);

		// example of sub-file that lifetime is smaller than the owner
		const auto materialsFileName = PathSystem::GetParentPath(m_path) + "\\" + PathSystem::GetNameWithoutExtension(m_path) + ".xmt";

		m_materialsXMLFile = GetDiskAssetManager()->LoadFileAsset<XMLDoc>(materialsFileName);

		if (!m_materialsXMLFile)
		{
			RT_XENGINE_LOG_WARN("Missing Materials file for model: \'{}\', convention expects: \'{}\'", m_path, materialsFileName);
			return false;
		}

		auto* root = m_materialsXMLFile->GetRootElement();

		// SetIdentificationFromAssociatedDiskAssetIdentification XMaterials from xmt file

		std::string mapsPathRootHint;
		Assets::ReadStringAttribute(root, "maps_path", mapsPathRootHint);

		for (auto* xmaterialElement = root->FirstChildElement(); xmaterialElement != nullptr;
			xmaterialElement = xmaterialElement->NextSiblingElement())
		{
			std::string matName;
			ReadStringAttribute(xmaterialElement, "name", matName);

			auto mat = std::make_shared<XMaterial>(this);

			// missing mat
			if (!mat->Load(xmaterialElement, mapsPathRootHint))
				return false;

			m_materials[matName] = mat;
		}

		for (auto i = 0u; i < meshCount; ++i)
		{
			auto mesh = std::make_shared<XMesh>(this);

			// missing mesh
			if (!mesh->Load(data))
				return false;

			m_meshes.emplace_back(mesh);
		}

		return true;
	}

	void XModel::Clear()
	{
		for (auto& me : m_meshes)
			me->Unload();

		for (auto& mat : m_materials)
			mat.second->Unload();

		m_materialsXMLFile->Unload();
	}
}
