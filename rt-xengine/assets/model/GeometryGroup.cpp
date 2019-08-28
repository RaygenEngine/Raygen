#include "pch.h"

#include "assets/model/GeometryGroup.h"
#include "assets/model/GltfAux.h"

namespace Assets
{
	GeometryGroup::GeometryGroup(DiskAsset* pAsset, const std::string& name)
		: DiskAssetPart(pAsset, name),
		  m_indicesBuffer(this, "indicesBuffer"),
		  m_positionsBuffer(this, "positionsBuffer"),
		  m_normalsBuffer(this, "normalsBuffer"),
		  m_tangentsBuffer(this, "tangentsBuffer"),
		  m_bitangentsBuffer(this, "bitangentsBuffer"),
		  m_textCoord0Buffer(this, "textCoord0Buffer"),
		  m_textCoord1Buffer(this, "textCoord1Buffer"),
		  m_color0Buffer(this, "color0Buffer"),
		  m_mode(GM_INVALID),
	      m_material(this, "default-mat")
	{
	}

	void GeometryGroup::LoadFromGltfData(const tinygltf::Model& modelData, const tinygltf::Primitive& primitiveData)
	{
		// mode
		m_mode = GetGeometryModeFromGltf(primitiveData.mode);
	
		// indexing
		const auto indicesIndex = primitiveData.indices;

		if (indicesIndex != -1)
		{
			auto& accessor = modelData.accessors.at(indicesIndex);
			m_indicesBuffer.LoadFromGltfData(modelData, accessor);
		}
		
		// attributes
		for (auto& attribute : primitiveData.attributes)
		{
			auto& accessor = modelData.accessors.at(attribute.second);
			
			if (Core::CaseInsensitiveCompare(attribute.first, "POSITION"))
				m_positionsBuffer.LoadFromGltfData(modelData, accessor);
			else if (Core::CaseInsensitiveCompare(attribute.first, "NORMAL"))
				m_normalsBuffer.LoadFromGltfData(modelData, accessor);
			else if (Core::CaseInsensitiveCompare(attribute.first, "TANGENT"))
				m_tangentsBuffer.LoadFromGltfData(modelData, accessor);
			else if (Core::CaseInsensitiveCompare(attribute.first, "TEXCOORD_0"))
				m_textCoord0Buffer.LoadFromGltfData(modelData, accessor);
			else if (Core::CaseInsensitiveCompare(attribute.first, "TEXCOORD_1"))
				m_textCoord1Buffer.LoadFromGltfData(modelData, accessor);
			else if (Core::CaseInsensitiveCompare(attribute.first, "COLOR_0"))
				m_color0Buffer.LoadFromGltfData(modelData, accessor);

		}

		// material
		const auto materialIndex = primitiveData.material;
		
		if (materialIndex != -1)
		{
			auto& mat = modelData.materials.at(materialIndex);
			m_material.LoadFromGltfData(modelData, mat);
			m_material.RenameAssetPart(Core::UnnamedDescription(mat.name));
		}

		// TODO calculate missing stuff (normals, tangents, bi-tangents, default mat) 
	}
}
