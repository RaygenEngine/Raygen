#pragma once

#include "assets/DiskAssetPart.h"
#include "assets/model/Material.h"
#include "assets/model/Buffer.h"

namespace tinygltf
{
	class Model;
	struct Primitive;
}

namespace Assets
{
	// Single Draw Call
	class GeometryGroup : public DiskAssetPart
	{
		Buffer m_indicesBuffer;

		Buffer m_positionsBuffer;
		Buffer m_normalsBuffer;
		Buffer m_tangentsBuffer;
		Buffer m_bitangentsBuffer;
		Buffer m_textCoord0Buffer;
		Buffer m_textCoord1Buffer;
		Buffer m_color0Buffer;

		// TODO joints/weights
		
		GeometryMode m_mode;

		// TODO fix this
		Material m_material;
		
	public:

		GeometryGroup(DiskAsset* pAsset, const std::string& name);
		
		void LoadFromGltfData(const tinygltf::Model& modelData, const tinygltf::Primitive& primitiveData);
		
		GeometryMode GetGeometryMode() const { return m_mode; }

		bool UsesIndexing() const { return !m_indicesBuffer.IsEmpty(); }

		void ToString(std::ostream& os) const override { os << "asset-type: GeometryGroup, name: " << m_name; }
	};

}
