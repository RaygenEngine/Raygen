#include "pch.h"
#include "XMesh.h"

#include "XModel.h"

namespace Assets
{

	XMesh::XMesh(DiskAsset* parent)
		: DiskAssetPart(parent)
	{
	}

	bool XMesh::Load(Core::XMDFileData& data)
	{
		ReadNullTerminatedStringFromBuffer(data, m_label);
		RT_XENGINE_LOG_TRACE("Loading XMesh, name: {}", m_label);

		uint32 faceCount;
		ReadValueLittleEndianFromBuffer(data, faceCount);
		RT_XENGINE_LOG_TRACE("Triangle Face count {0}", faceCount);

		m_vertexIndices.resize(faceCount);
		ReadVectorLittleEndianFromBuffer(data, m_vertexIndices);

		m_materialIndices.resize(faceCount);
		ReadVectorLittleEndianFromBuffer(data, m_materialIndices);

		uint32 vertexCount;
		ReadValueLittleEndianFromBuffer(data, vertexCount);
		RT_XENGINE_LOG_TRACE("Vertex count {0}", vertexCount);

		m_vertices.resize(vertexCount);
		ReadVectorLittleEndianFromBuffer(data, m_vertices);

		uint32 materialCount;
		ReadValueLittleEndianFromBuffer(data, materialCount);
		RT_XENGINE_LOG_TRACE("Material count: {0}", materialCount);

		for (uint32 i = 0; i < materialCount; ++i)
		{
			std::string matName;
			ReadNullTerminatedStringFromBuffer(data, matName);
			// for now
			auto mat = dynamic_cast<XModel*>(GetParent())->GetMaterialByName(matName);
			m_materials.push_back(mat);
		}

		CalculateMaterialOffsets();

		MarkLoaded();

		return true;
	}

	void XMesh::Clear()
	{
		std::vector<Core::Vertex>().swap(m_vertices);
		std::vector<glm::u32vec3>().swap(m_vertexIndices);
		std::vector<uint32>().swap(m_materialIndices);
		std::vector<std::pair<uint32, uint32>>().swap(m_materialOffsets);

		// unload part assets from inside their hosts
		for (auto& mat : m_materials)
			mat->Unload();
	}

	std::vector<XMaterial*> XMesh::GetMaterialsInOffsetOrder() const
	{
		std::vector<XMaterial*> materialsOffsetOrder;
		for (auto& matOffset : m_materialOffsets)
		{
			                                                            // first and second are both inclusive 
			materialsOffsetOrder.push_back(m_materials.at(m_materialIndices.at(matOffset.first)).get());
		}

		return materialsOffsetOrder;
	}

	void XMesh::CalculateMaterialOffsets()
	{
		uint32 lastMatIndex = m_materialIndices[0];
		uint32 lastPos = 0;
		for (uint32 i = 0; i < m_materialIndices.size(); ++i)
		{
			if (lastMatIndex != m_materialIndices[i])
			{
				// index where different mat begins
				m_materialOffsets.emplace_back(lastPos, i - 1); 
				// keep last inclusive
				lastMatIndex = m_materialIndices[i];
				lastPos = i;
			}
		}
		m_materialOffsets.emplace_back(lastPos, static_cast<uint32>(m_materialIndices.size() - 1));

		// this should not happen as XMesh faces are in material order
		if (m_materials.size() != m_materialOffsets.size())
			RT_XENGINE_LOG_WARN("Material offsets > material count ({} > {}),  If faces are in material order you can optimize draw calls in some APIs, XMesh name: {}", m_materialOffsets.size(), m_materials.size(), m_label);
	}
}
