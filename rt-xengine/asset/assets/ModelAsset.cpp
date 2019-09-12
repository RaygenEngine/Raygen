#include "pch.h"

#include "asset/assets/ModelAsset.h"
#include "asset/AssetManager.h"
#include "system/Engine.h"
#include "asset/util/GltfAux.h"

#include "tinygltf/tiny_gltf.h"
#include <optional>

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
	template<> void CopyToVector<float>(std::vector<uint32>&, byte*, size_t, size_t, size_t) { assert(false); }
	template<> void CopyToVector<double>(std::vector<uint32>&, byte*, size_t, size_t, size_t) { assert(false); }
	template<> void CopyToVector<byte>(std::vector<uint32>&, byte*, size_t, size_t, size_t) { assert(false); }

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


			componentType = GltfAux::GetComponentType(accessor.componentType);
			elementCount = accessor.count;
			beginByteOffset = accessor.byteOffset + bufferView.byteOffset;
			strideByteOffset = accessor.ByteStride(bufferView);
			componentCount = utl::GetElementComponentCount(GltfAux::GetElementType(accessor.type));
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

	bool LoadSampler(SamplerPod& outPod, const tg::Model& model, const tg::Texture& textureData, bool loadDefaultIfMissing = true)
	{
		bool didLoad = false;
		const auto imageIndex = textureData.source;

		// if image exists
		if (imageIndex != -1)
		{
			// TODO check image settings
			auto& gltfImage = model.images.at(imageIndex);

			outPod.image = Engine::GetAssetManager()->RequestSearchAsset<ImageAsset>(gltfImage.uri);
			didLoad = true;
		}

		const auto samplerIndex = textureData.sampler;
		// if sampler exists
		if (samplerIndex != -1)
		{
			auto& gltfSampler = model.samplers.at(samplerIndex);

			outPod.minFilter = GltfAux::GetTextureFiltering(gltfSampler.minFilter);
			outPod.magFilter = GltfAux::GetTextureFiltering(gltfSampler.magFilter);
			outPod.wrapS = GltfAux::GetTextureWrapping(gltfSampler.wrapS);
			outPod.wrapT = GltfAux::GetTextureWrapping(gltfSampler.wrapT);
			outPod.wrapR = GltfAux::GetTextureWrapping(gltfSampler.wrapR);
		}
		//else keep default values
		return didLoad;
	}


	bool LoadMaterial(MaterialPod& outMaterial, tg::Model& model, const tg::Material& materialData)
	{
		// factors
		auto bFactor = materialData.pbrMetallicRoughness.baseColorFactor;
		outMaterial.baseColorFactor = { bFactor[0], bFactor[1], bFactor[2], bFactor[3] };
		outMaterial.metallicFactor = static_cast<float>(materialData.pbrMetallicRoughness.metallicFactor);
		outMaterial.roughnessFactor = static_cast<float>(materialData.pbrMetallicRoughness.roughnessFactor);
		auto eFactor = materialData.emissiveFactor;
		outMaterial.emissiveFactor = { eFactor[0], eFactor[1], eFactor[2] };

		// scales/strenghts
		outMaterial.normalScale = static_cast<float>(materialData.normalTexture.scale);
		outMaterial.occlusionStrength = static_cast<float>(materialData.occlusionTexture.strength);

		// alpha
		outMaterial.alphaMode = GltfAux::GetAlphaMode(materialData.alphaMode);

		outMaterial.alphaCutoff = static_cast<float>(materialData.alphaCutoff);
		// doublesided-ness
		outMaterial.doubleSided = materialData.doubleSided;


		auto LoadTexture = [&](auto textureInfo, SamplerPod& sampler, int32& textCoordIndex, bool useDefaultIfMissing = true)
		{
			bool didLoad = false;
			if (textureInfo.index != -1)
			{
				tg::Texture& gltfTexture = model.textures.at(textureInfo.index);
				
				didLoad = LoadSampler(sampler, model, gltfTexture, useDefaultIfMissing);

				textCoordIndex = textureInfo.texCoord;
			}
			
			if (useDefaultIfMissing && !didLoad)
			{
				sampler.image = AssetManager::GetDefaultWhite();
			}

			return true;
		};

		// samplers
		auto& baseColorTextureInfo = materialData.pbrMetallicRoughness.baseColorTexture;
		LoadTexture(baseColorTextureInfo, outMaterial.baseColorTexture, outMaterial.baseColorTexCoordIndex);

		auto& emissiveTextureInfo = materialData.emissiveTexture;
		LoadTexture(emissiveTextureInfo, outMaterial.emissiveTexture, outMaterial.emissiveTexCoordIndex);

		auto& normalTextureInfo = materialData.normalTexture;
		LoadTexture(normalTextureInfo, outMaterial.normalTexture, outMaterial.normalTexCoordIndex, false);

		// TODO: pack if different
		auto& metallicRougnessTextureInfo = materialData.pbrMetallicRoughness.metallicRoughnessTexture;
		auto& occlusionTextureInfo = materialData.occlusionTexture;

		// same texture no need of packing
		//if(metallicRougnessTextureInfo.index == occlusionTextureInfo.index)
		{
			LoadTexture(metallicRougnessTextureInfo, outMaterial.occlusionMetallicRoughnessTexture, outMaterial.occlusionMetallicRoughnessTexCoordIndex);
		}
		return true;
	}

	bool LoadGeometryGroup(GeometryGroupPod& geom, tinygltf::Model& modelData, const tinygltf::Primitive& primitiveData,
		const glm::mat4& transformMat)
	{
		// mode
		geom.mode = GltfAux::GetGeometryMode(primitiveData.mode);

		// indexing
		const auto indicesIndex = primitiveData.indices;

		if (indicesIndex != -1)
		{
			ExtractBufferDataInto(modelData, indicesIndex, geom.indices);
		}

		// attributes
		for (auto& attribute : primitiveData.attributes)
		{
			const auto& attrName = attribute.first;
			int32 index = attribute.second;

			if (utl::CaseInsensitiveCompare(attrName, "POSITION"))
			{
				ExtractBufferDataInto(modelData, index, geom.positions);
			}
			else if (utl::CaseInsensitiveCompare(attrName, "NORMAL"))
			{
				ExtractBufferDataInto(modelData, index, geom.normals);
			}
			else if (utl::CaseInsensitiveCompare(attrName, "TANGENT"))
			{
				ExtractBufferDataInto(modelData, index, geom.tangents);
			}
			else if (utl::CaseInsensitiveCompare(attrName, "TEXCOORD_0"))
			{
				ExtractBufferDataInto(modelData, index, geom.textCoords0);
			}
			else if (utl::CaseInsensitiveCompare(attrName, "TEXCOORD_1"))
			{
				ExtractBufferDataInto(modelData, index, geom.textCoords1);
			}

		}

		// if missing positions
		if (geom.positions.empty())
			return false;

		// PERF: speed up those calcs
		for (auto& pos : geom.positions)
		{
			pos = transformMat * glm::vec4(pos, 1.f);
		}
		// PERF: speed up those calcs
		const auto invTransMat = glm::transpose(glm::inverse(glm::mat3(transformMat)));
		for (auto& normal : geom.normals)
		{
			normal = invTransMat * normal;
		}

		// material
		const auto materialIndex = primitiveData.material;

		if (materialIndex != -1)
		{
			auto& mat = modelData.materials.at(materialIndex);
			if (!LoadMaterial(geom.material, modelData, mat))
			{
				return false;
			}
		}

		// calculate missing normals (flat)
		if (geom.normals.empty())
		{
			geom.normals.resize(geom.positions.size());

			auto makeNormals = [&](std::function<uint32(int32)> getIndex)
			{
				for (int32 i = 0; i < geom.indices.size(); i += 3)
				{
					// triangle
					auto p0 = geom.positions[getIndex(i)];
					auto p1 = geom.positions[getIndex(i + 1)];
					auto p2 = geom.positions[getIndex(i + 2)];

					glm::vec3 n = glm::cross(p1 - p0, p2 - p0);

					geom.normals[getIndex(i)] += n;
					geom.normals[getIndex(i + 1)] += n;
					geom.normals[getIndex(i + 2)] += n;
				}
			};

			if (!geom.indices.empty())
			{
				makeNormals([&](int32 i) { return geom.indices[i]; });
			}
			else
			{
				makeNormals([](int32 i) { return i; });
			}

			std::for_each(geom.normals.begin(), geom.normals.end(), [](glm::vec3& normal) { normal = glm::normalize(normal); });
		}

		// TODO test better calculations (using uv layer 0?) also text tangent handedness
		// calculate missing tangents (and bitangents)
		if (geom.tangents.empty())
		{
			std::transform(geom.normals.begin(), geom.normals.end(), std::back_inserter(geom.tangents), [](const glm::vec3& normal)
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
		if (geom.bitangents.empty())
		{
			std::transform(geom.normals.begin(), geom.normals.end(), geom.tangents.begin(),
				std::back_inserter(geom.bitangents), [](const glm::vec3& normal, const glm::vec4& tangent)
				{
					return glm::normalize(glm::cross(normal, glm::vec3(tangent)) * tangent.w);
				});
		}

		// calculate missing textCoords0 - init zeros
		if (geom.textCoords0.empty())
		{
			geom.textCoords0.resize(geom.positions.size());
		}

		// calculate missing textCoords1 - copy textCoords0
		if (geom.textCoords1.empty())
		{
			geom.textCoords1 = geom.textCoords0;
		}

		// calculate other baked data
		return true;
	}


	bool LoadMesh(MeshPod& mesh, tinygltf::Model& modelData, const tinygltf::Mesh& meshData, const glm::mat4& transformMat)
	{
		
		mesh.geometryGroups.resize(meshData.primitives.size());

		// primitives
		for (int32 i = 0; i < mesh.geometryGroups.size(); ++i)
		{
			const auto geomName = "geom_group" + std::to_string(i);

			auto& primitiveData = meshData.primitives.at(i);

			// if one of the geometry groups fails to load
			if (!LoadGeometryGroup(mesh.geometryGroups[i], modelData, primitiveData, transformMat))
			{
				LOG_ERROR("Failed to load geometry group, name: {}", geomName);
				return false;
			}
		}
		return true;
	}


}

bool ModelAsset::LoadFromGltfImpl()
{
	namespace tg = tinygltf;

	tg::TinyGLTF loader;
	tg::Model model; 
	
	std::string err;
	std::string warn;

	auto ext = m_uri.extension();

	bool ret = false;


	if (ext.compare(".gltf") == 0)
	{
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, m_uri.string());
	}

	CLOG_WARN(!warn.empty(), warn.c_str());
	CLOG_ERROR(!err.empty(), err.c_str());

	if (!ret)
	{
		return false;
	}

	auto& defaultScene = model.scenes.at(model.defaultScene);

	std::function<bool(const std::vector<int>&, glm::mat4)> RecurseChildren;
	RecurseChildren = [&](const std::vector<int>& childrenIndices, glm::mat4 parentTransformMat)
	{
		for (auto& nodeIndex : childrenIndices)
		{
			auto& childNode = model.nodes.at(nodeIndex);

			glm::mat4 localTransformMat = glm::mat4(1.f);

			// When matrix is defined, it must be decomposable to TRS.
			if (!childNode.matrix.empty())
			{
				for (int32 row = 0; row < 4; ++row)
					for (int32 column = 0; column < 4; ++column)
						localTransformMat[row][column] = static_cast<float>(childNode.matrix[column + 4 * row]);
			}
			else
			{
				glm::vec3 translation = glm::vec3(0.f);
				glm::quat orientation = { 1.f, 0.f, 0.f, 0.f };
				glm::vec3 scale = glm::vec3(1.f);

				if (!childNode.translation.empty())
				{
					translation[0] = static_cast<float>(childNode.translation[0]);
					translation[1] = static_cast<float>(childNode.translation[1]);
					translation[2] = static_cast<float>(childNode.translation[2]);
				}

				if (!childNode.rotation.empty())
				{
					orientation[0] = static_cast<float>(childNode.rotation[0]);
					orientation[1] = static_cast<float>(childNode.rotation[1]);
					orientation[2] = static_cast<float>(childNode.rotation[2]);
					orientation[3] = static_cast<float>(childNode.rotation[3]);
				}

				if (!childNode.scale.empty())
				{
					scale[0] = static_cast<float>(childNode.scale[0]);
					scale[1] = static_cast<float>(childNode.scale[1]);
					scale[2] = static_cast<float>(childNode.scale[2]);
				}

				localTransformMat = utl::GetTransformMat(translation, orientation, scale);
			}

			localTransformMat = parentTransformMat * localTransformMat;

			// TODO: check instancing cases
			// load mesh if exists
			if (childNode.mesh != -1)
			{
				auto& gltfMesh = model.meshes.at(childNode.mesh);

				MeshPod mesh;

				// if missing mesh
				if (!LoadMesh(mesh, model, gltfMesh, localTransformMat))
				{
					LOG_ERROR("Failed to load mesh, name: {}", gltfMesh.name);
					return false;
				}
				m_meshes.emplace_back(mesh);
			}

			//load child's children
			if (!childNode.children.empty())
				if (!RecurseChildren(childNode.children, localTransformMat))
					return false;
		}
		return true;
	};

	return RecurseChildren(defaultScene.nodes, glm::mat4(1.f));
}


bool ModelAsset::Load()
{
	if (m_uri.extension().compare(".gltf") == 0)
	{
		TIMER_STATIC_SCOPE("gltf model load");
		return LoadFromGltfImpl();
	}
	return true;
}

void ModelAsset::Unload()
{
}
