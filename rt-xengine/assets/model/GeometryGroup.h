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
		// TODO uint32
		std::vector<uint32> m_indices;


		std::vector<glm::vec3> m_positions;
		std::vector<glm::vec3> m_normals;
		std::vector<glm::vec4> m_tangents;
		std::vector<glm::vec3> m_bitangents;
		std::vector<glm::vec2> m_textCoords0;
		std::vector<glm::vec2> m_textCoords1;

		// TODO joints/weights
		
		GeometryMode m_mode;

		Material m_material;
		
	public:

		GeometryGroup(DiskAsset* pAsset, const std::string& name);
		
		bool Load(const tinygltf::Model& modelData, const tinygltf::Primitive& primitiveData, const glm::mat4& transformMat);
		
		GeometryMode GetGeometryMode() const { return m_mode; }

		const Material& GetMaterial() const { return m_material; }

		bool UsesIndexing() const { return !m_indices.empty(); }

		const std::vector<glm::uint32>& GetIndices() const { return m_indices; }
		const std::vector<glm::vec3>& GetPositions() const { return m_positions; }
		const std::vector<glm::vec3>& GetNormals() const { return m_normals; }
		const std::vector<glm::vec4>& GetTangents() const { return m_tangents; }
		const std::vector<glm::vec3>& GetBitangents() const { return m_bitangents; }
		const std::vector<glm::vec2>& GetTextCoords0() const { return m_textCoords0; }
		const std::vector<glm::vec2>& GetTextCoords1() const { return m_textCoords1; }

		void ToString(std::ostream& os) const override { os << "asset-type: GeometryGroup, name: " << m_name; }
	};

}
