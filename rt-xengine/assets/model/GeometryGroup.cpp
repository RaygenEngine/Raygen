#include "pch.h"

#include "assets/model/GeometryGroup.h"
#include "assets/model/GltfAux.h"

namespace Assets
{
	GeometryGroup::GeometryGroup(DiskAsset* pAsset, const std::string& name)
		: DiskAssetPart(pAsset, name),
		  m_mode(GeometryMode::TRIANGLES),
	      m_material(this, "default-mat")
	{
	}
	
	
	namespace
	{
		namespace tg = tinygltf;

		// TODO: This code has high maintenance cost due to its complexity.
		// the complexity here is mostly used to protect against 'incompatible' conversion types. 
		// The maintenance cost may not be worth the 'safe' conversions.

		// Copy with strides from a C array-like to a vector while performing normal type conversion.
		template<typename ComponentType, typename OutputType>
		void CopyToVector(std::vector<OutputType>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount, size_t componentCount)
		{
			for (int32 i = 0; i < elementCount; ++i)
			{
				byte* elementPtr = &beginPtr[perElementOffset * i];
				ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);

				for (uint32 c = 0; c < componentCount; ++c)
				{
					if constexpr (std::is_same_v<double, ComponentType>)
					{
						result[i][c] = static_cast<float>(data[c]); // explicitly convert any double input to float.
					}
					else 
					{
						result[i][c] = data[c]; // normal type conversion, should be able to convert most of the required stuff
					}
				}
			}
		}

		// Empty specialization for conversions between types that should never happen.
		template<> void CopyToVector<uint32, glm::vec2>(std::vector<glm::vec2>&, byte*, size_t, size_t, size_t) { assert(false); }
		template<> void CopyToVector<uint32, glm::vec3>(std::vector<glm::vec3>&, byte*, size_t, size_t, size_t) { assert(false); }
		template<> void CopyToVector<uint32, glm::vec4>(std::vector<glm::vec4>&, byte*, size_t, size_t, size_t) { assert(false); }

		// Uint16 specialization. Expects componentCount == 1.
		template<>
		void CopyToVector<unsigned short>(std::vector<uint32>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount, size_t componentCount)
		{
			assert(componentCount == 1);
			for (uint32 i = 0; i < elementCount; ++i)
			{
				byte* elementPtr = &beginPtr[perElementOffset * i];
				unsigned short* data = reinterpret_cast<unsigned short*>(elementPtr);
				result[i] = *data;
			}
		}

		// Uint32 specialization. Expects componentCount == 1.
		template<>
		void CopyToVector<unsigned int>(std::vector<uint32>& result, byte* beginPtr, size_t perElementOffset, size_t elementCount, size_t componentCount)
		{
			assert(componentCount == 1);
			for (uint32 i = 0; i < elementCount; ++i)
			{
				byte* elementPtr = &beginPtr[perElementOffset * i];
				unsigned int* data = reinterpret_cast<unsigned int*>(elementPtr);
				result[i] = *data;
			}
		}
		
		// Uint32 empty specializations. runtime assert just in case
		template<> void CopyToVector<float> (std::vector<uint32>&, byte*, size_t, size_t, size_t) { assert(false); }
		template<> void CopyToVector<double>(std::vector<uint32>&, byte*, size_t, size_t, size_t) { assert(false); }
		template<> void CopyToVector<byte>  (std::vector<uint32>&, byte*, size_t, size_t, size_t) { assert(false); }


		// Extracts the type of the out vector and attempts to load any type of data at accessorIndex to this vector.
		template<typename Output>
		void ExtractBufferDataInto(const tg::Model& modelData, int32 accessorIndex, std::vector<Output>& out)
		{
			//
			// Actual example of a possible complex gltf buffer:
			//                                              |     STRIDE  |
			// [{vertexIndexes} * 1000] [{normals} * 1000] [{uv0, position} * 1000]
			//													  ^ beginPtr for Position.
			//

			size_t elementCount;		// How many elements there are to read
			size_t componentCount;		// How many components of type ComponentType there are to each element.

			size_t strideByteOffset;	// The number of bytes to move in the buffer after each read to get the next element.
										// This may be more bytes than the actual sizeof(ComponentType) * componentCount
										// if the data is strided.

			byte* beginPtr;				// Pointer to the first byte we care about.
										// This may not be the actual start of the buffer of the binary file.

			BufferComponentType componentType; // this particular model's underlying buffer type to read as.

			{
				size_t beginByteOffset;
				const tinygltf::Accessor& accessor = modelData.accessors.at(accessorIndex);
				const tinygltf::BufferView& bufferView = modelData.bufferViews.at(accessor.bufferView);
				const tinygltf::Buffer& gltfBuffer = modelData.buffers.at(bufferView.buffer);


				componentType = GetComponentTypeFromGltf(accessor.componentType);
				elementCount = accessor.count;
				beginByteOffset = accessor.byteOffset + bufferView.byteOffset;
				strideByteOffset = accessor.ByteStride(bufferView);
				componentCount = GetElementComponentCount(GetElementTypeFromGltf(accessor.type));
				beginPtr = const_cast<byte*>(&gltfBuffer.data[beginByteOffset]);
			}

			out.resize(elementCount);

			// This will generate EVERY possible mapping of Output -> ComponentType conversion
			// fix this later and provide empty specializations of "incompatible" types (eg: float -> int)
			// TODO: This code will produce warnings for every type conversion that is considered 'unsafe'

			switch (componentType)
			{
			// Conversions from signed to unsigned types are "implementation defined".
			// This code assumes the implementation will not do any bit arethmitic from signed x to unsigned x.

			case BufferComponentType::BYTE:				// Fallthrough
			case BufferComponentType::UNSIGNED_BYTE:
				CopyToVector<unsigned char>(out, beginPtr, strideByteOffset, elementCount, componentCount);
				return;
			case BufferComponentType::SHORT:			// Fallthrough 
			case BufferComponentType::UNSIGNED_SHORT:
				CopyToVector<unsigned short>(out, beginPtr, strideByteOffset, elementCount, componentCount);
				return;
			case BufferComponentType::INT:
			case BufferComponentType::UNSIGNED_INT:		// Fallthrough
				CopyToVector<uint32>(out, beginPtr, strideByteOffset, elementCount, componentCount);
				return;
			case BufferComponentType::FLOAT:
				CopyToVector<float>(out, beginPtr, strideByteOffset, elementCount, componentCount);
				return;
			case BufferComponentType::DOUBLE:
				CopyToVector<double>(out, beginPtr, strideByteOffset, elementCount, componentCount);
				return;
			case BufferComponentType::INVALID:
				return;
			}
			return;
		}
	}
	
	bool GeometryGroup::Load(const tinygltf::Model& modelData, const tinygltf::Primitive& primitiveData, const glm::mat4& transformMat)
	{
		// mode
		m_mode = GetGeometryModeFromGltf(primitiveData.mode);		
		
		// indexing
		const auto indicesIndex = primitiveData.indices;

		if (indicesIndex != -1)
		{
			ExtractBufferDataInto(modelData, indicesIndex, m_indices);
		}

		// attributes
		for (auto& attribute : primitiveData.attributes)
		{
			const auto& attrName = attribute.first;
			int32 index = attribute.second;

			if (Core::CaseInsensitiveCompare(attrName, "POSITION"))
			{
				ExtractBufferDataInto(modelData, index, m_positions);
			}
			else if (Core::CaseInsensitiveCompare(attrName, "NORMAL"))
			{
				ExtractBufferDataInto(modelData, index, m_normals);
			}
			else if (Core::CaseInsensitiveCompare(attrName, "TANGENT"))
			{
				ExtractBufferDataInto(modelData, index, m_tangents);
			}
			else if (Core::CaseInsensitiveCompare(attrName, "TEXCOORD_0"))
			{
				ExtractBufferDataInto(modelData, index, m_textCoords0);
			}
			else if (Core::CaseInsensitiveCompare(attrName, "TEXCOORD_1"))
			{
				ExtractBufferDataInto(modelData, index, m_textCoords1);
			}

		}


		// TODO: speed up those calcs
		for(auto& pos : m_positions)
		{
			pos = transformMat * glm::vec4(pos,1.f);
		}

		const auto invTransMat = glm::transpose(glm::inverse(glm::mat3(transformMat)));
		for (auto& normal : m_normals)
		{
			normal = invTransMat * normal;
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
			
			auto makeNormals = [&](std::function<uint32(int32)> getIndex)
			{
				for (int32 i = 0; i < m_indices.size(); i += 3)
				{
					// triangle
					auto p0 = m_positions[getIndex(i)];
					auto p1 = m_positions[getIndex(i + 1)];
					auto p2 = m_positions[getIndex(i + 2)];

					glm::vec3 n = glm::cross(p1 - p0, p2 - p0);

					m_normals[getIndex(i)] += n;
					m_normals[getIndex(i + 1)] += n;
					m_normals[getIndex(i + 2)] += n;
				}
			};

			if(UsesIndexing())
			{
				makeNormals([&](int32 i) { return m_indices[i]; });
			}
			else
			{
				makeNormals([](int32 i) { return i; });
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
