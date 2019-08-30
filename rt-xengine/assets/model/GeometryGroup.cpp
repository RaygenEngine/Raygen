#include "pch.h"

#include "assets/model/GeometryGroup.h"
#include "assets/model/GltfAux.h"

namespace Assets
{
	GeometryGroup::GeometryGroup(DiskAsset* pAsset, const std::string& name)
		: DiskAssetPart(pAsset, name),
		  m_mode(GM_INVALID),
	      m_material(this, "default-mat")
	{
	}

	bool GeometryGroup::Load(const tinygltf::Model& modelData, const tinygltf::Primitive& primitiveData)
	{
		// mode
		m_mode = GetGeometryModeFromGltf(primitiveData.mode);		
		const auto CopyBufferDataFromGltfAccessor = [&](int32 accessorIndex, auto& target)
		{
			auto& accessor = modelData.accessors.at(accessorIndex);
			auto& bufferView = modelData.bufferViews.at(accessor.bufferView);
			auto& buffer = modelData.buffers.at(bufferView.buffer);

			auto elementType = GetElementTypeFromGltf(accessor.type);
			auto componentType = GetComponentTypeFromGltf(accessor.componentType);

			const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;

			auto componentCount = GetElementComponentCount(elementType);
			auto elementBytes = (GetComponentTypeByteCount(componentType) * GetElementComponentCount(elementType));
			auto elementCount = accessor.count;

			target.resize(elementCount);

			uint64 strideOffset = 0;
			uint64 dataOffset = 0;
			for (uint64 i = 0; i < elementCount; ++i, dataOffset += elementBytes, strideOffset += bufferView.byteStride)
			{
				for (auto c = 0; c < componentCount; ++c)
				{
					switch (componentType)
					{
					case BCT_UNSIGNED_BYTE:
						target[i][c] = (&buffer.data[byteOffset + dataOffset + strideOffset])[c];
						break;
					case BCT_UNSIGNED_SHORT:
						target[i][c] = reinterpret_cast<const uint16*>(&buffer.data[byteOffset + dataOffset + strideOffset])[c];
						break;
					case BCT_UNSIGNED_INT:
						target[i][c] = reinterpret_cast<const uint32*>(&buffer.data[byteOffset + dataOffset + strideOffset])[c];
						break;
					case BCT_FLOAT:
						target[i][c] = reinterpret_cast<const float*>(&buffer.data[byteOffset + dataOffset + strideOffset])[c];
						break;
					default:
						RT_XENGINE_ASSERT(false, "wrong model");
					}
				}
			}
		};
		
		// indexing
		const auto indicesIndex = primitiveData.indices;

		if (indicesIndex != -1)
		{
			CopyBufferDataFromGltfAccessor(indicesIndex, m_indices);
		}
		
		// attributes
		for (auto& attribute : primitiveData.attributes)
		{
			if (Core::CaseInsensitiveCompare(attribute.first, "POSITION"))
				CopyBufferDataFromGltfAccessor(attribute.second, m_positions);
			else if (Core::CaseInsensitiveCompare(attribute.first, "NORMAL"))
				CopyBufferDataFromGltfAccessor(attribute.second, m_normals);
			else if (Core::CaseInsensitiveCompare(attribute.first, "TANGENT"))
				CopyBufferDataFromGltfAccessor(attribute.second, m_tangents);
			else if (Core::CaseInsensitiveCompare(attribute.first, "TEXCOORD_0"))
				CopyBufferDataFromGltfAccessor(attribute.second, m_textCoords0);
			else if (Core::CaseInsensitiveCompare(attribute.first, "TEXCOORD_1"))
				CopyBufferDataFromGltfAccessor(attribute.second, m_textCoords1);
		}

		// material
		const auto materialIndex = primitiveData.material;
		
		if (materialIndex != -1)
		{
			auto& mat = modelData.materials.at(materialIndex);

			m_material.Load(modelData, mat);
			m_material.RenameAssetPart(Core::UnnamedDescription(mat.name));
		}

		// if missing positions it fails
		if (m_positions.empty())
			return false;

		// calculate missing normals (flat)
		if (m_normals.empty())
		{
			m_normals.resize(m_positions.size());
			
			if(UsesIndexing())
			{
				for (int32 i = 0; i < m_indices.size(); i+=3)
				{					
					// triangle
					auto p0 = m_positions[m_indices[i].x];
					auto p1 = m_positions[m_indices[i+1].x];
					auto p2 = m_positions[m_indices[i+2].x];
					
					glm::vec3 n = glm::cross(p1 - p0, p2 - p0);
					
					m_normals[m_indices[i].x] += n;
					m_normals[m_indices[i+1].x] += n;
					m_normals[m_indices[i+2].x] += n;
				}	
			}
			else
			{
				for (int32 i = 0; i < m_positions.size(); ++i)
				{					
					// triangle
					auto p0 = m_positions[i];
					auto p1 = m_positions[i+1];
					auto p2 = m_positions[i+2];

					glm::vec3 n = glm::cross(p1 - p0, p2 - p0);    // p1 is the 'base' here

					m_normals[i] += n;
					m_normals[i+1] += n;
					m_normals[i+2] += n;
				}
			}

			std::for_each(m_normals.begin(), m_normals.end(), [](glm::vec3& normal) { normal = glm::normalize(normal); });
		}

		// TODO test better calculations (using uv layer 0?) also text tangent handedness
		// calculate missing tangents (and bitangents)
		if (m_tangents.empty())
		{
			std::transform(m_normals.begin(), m_normals.end(), std::back_inserter(m_tangents), [](const glm::vec3& normal)
				{
					const auto c1 = glm::cross(normal, glm::vec3(0.0, 0.0, 1.0));
					const auto c2 = glm::cross(normal, glm::vec3(0.0, 1.0, 0.0));
					if (glm::length(c1) > glm::length(c2))
						return glm::vec4(glm::normalize(c1), 1.0f);
					else
						return glm::vec4(glm::normalize(c2), -1.f);
				});
		}

		// calculate missing bitangents
		if (m_bitangents.empty())
		{
			std::transform(m_normals.begin(), m_normals.end(), m_tangents.begin(),
				std::back_inserter(m_bitangents), [](const glm::vec3& normal, const glm::vec4& tangent)
				{
					return glm::normalize(glm::cross(normal, glm::vec3(tangent)) * tangent.w);
				});
		}

		// calculate missing textCoords0 - init zeros
		if (m_textCoords0.empty())
		{
			m_textCoords0.resize(m_positions.size());
		}

		// calculate missing textCoords1 - copy textCoords0
		if (m_textCoords1.empty())
		{
			m_textCoords1 = m_textCoords0;
		}

		// calculate other baked data
		return true;
	}
}
