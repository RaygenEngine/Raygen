#pragma once

#include "assets/Asset.h"
#include "assets/texture/Texture.h"

#include <optional>

namespace tinygltf
{
	struct Mesh;
	struct Primitive;
	struct Material;
	class Model;
}

// glTF-based model (not all extensions included) 
class Model : public Asset
{
public:

	struct Sampler
	{
		std::string name;
		
		std::shared_ptr<Texture> texture = nullptr;

		TextureFiltering minFilter = TextureFiltering::LINEAR;
		TextureFiltering magFilter = TextureFiltering::LINEAR;

		TextureWrapping wrapS = TextureWrapping::REPEAT;
		TextureWrapping wrapT = TextureWrapping::REPEAT;
		TextureWrapping wrapR = TextureWrapping::REPEAT;

		int32 texCoordIndex = 0;
	};
	
	// Note: assets of this class (Textures) are not cached directly as they are part of a cached Model anyway
	// glTF-based material (not all extensions included) (comments in this file -> https://github.com/KhronosGroup/glTF/tree/master/specification/2.0)
	struct Material
	{
		std::string name;
		
		// The value for each property(baseColor, metallic, roughness) can be defined using factors or textures.

		// If a texture is not given, all respective texture components within this material model are assumed to have a value of 1.0.
		// If both factors and textures are present the factor value acts as a linear multiplier for the corresponding texture values.
		// The baseColorTexture uses the sRGB transfer function and must be converted to linear space before it is used for any computations.

		// The base color has two different interpretations depending on the value of metalness.
		// When the material is a metal, the base color is the specific measured reflectance value at normal incidence (F0).
		// For a non-metal the base color represents the reflected diffuse color of the material.
		// In this model it is not possible to specify a F0 value for non-metals, and a linear value of 4% (0.04) is used.
		// The baseColorTexture uses the sRGB transfer function and must be converted to linear space before it is used for any computations.
		// R-red, G-green, B-blue, A-alpha
		Sampler baseColorTextureSampler;
		// The metallic and roughness properties are packed together in a single texture called metallicRoughnessTexture.
		// R-empty, G-roughness, B-metal, A-empty
		Sampler metallicRoughnessTextureSampler;
		// A tangent space normal map
		Sampler normalTextureSampler;
		// The occlusion map indicating areas of indirect lighting
		Sampler occlusionTextureSampler;
		// The emissive map controls the color and intensity of the light being emitted by the material.
		Sampler emissiveTextureSampler;

		// Factor values act as linear multipliers for the corresponding texture values.
		glm::vec4 baseColorFactor = { 1.f, 1.f, 1.f, 1.f };
		glm::vec3 emissiveFactor = { 0.f, 0.f, 0.f };
		float metallicFactor = 1.f;
		float roughnessFactor = 1.f;

		// scaledNormal = normalize((<sampled normal texture value> * 2.0 - 1.0) * vec3(<normal scale>, <normal scale>, 1.0))
		float normalScale = 1.f;
		// occludedColor = lerp(color, color * <sampled occlusion texture value>, <occlusion strength>)
		float occlusionStrength = 1.f;

		// When alphaMode is set to MASK the alphaCutoff property specifies the cutoff threshold. If the alpha value is greater than or equal
		// to the alphaCutoff value then it is rendered as fully opaque, otherwise, it is rendered as fully transparent. alphaCutoff value is ignored for other modes.
		// The alpha value is defined in the baseColorTexture for metallic-roughness material model.
		AlphaMode alphaMode = AM_OPAQUE;
		float alphaCutoff = 0.5f;

		// The doubleSided property specifies whether the material is double sided. When this value is false, back-face culling is enabled. When this value is true,
		// back-face culling is disabled and double sided lighting is enabled. The back-face must have its normals reversed before the lighting equation is evaluated.
		bool doubleSided = false;
	};
	
	struct GeometryGroup
	{
		std::string name;
		
		std::vector<uint32> indices;

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec4> tangents;
		std::vector<glm::vec3> bitangents;
		std::vector<glm::vec2> textCoords0;
		std::vector<glm::vec2> textCoords1;

		// TODO joints/weights

		GeometryMode mode = GeometryMode::TRIANGLES;

		Material material;
	};

	
	struct Mesh
	{
		std::string name;
		
		std::vector<GeometryGroup> geometryGroups;
	};
	
	GeometryUsage m_usage;
	
	struct Info
	{
		std::string version;
		std::string generator;
		std::string minVersion;
		std::string copyright;
	} m_info;


private:
	std::vector<Mesh> m_meshes;

	template<bool LoadDefault>
	Sampler LoadSampler(const tinygltf::Model& modelData, int32 gltfTextureIndex, int32 gltfTexCoordTarget);
	Material LoadMaterial(const tinygltf::Model& modelData, const tinygltf::Material& materialData);
	std::optional<GeometryGroup> LoadGeometryGroup(const tinygltf::Model& modelData, const tinygltf::Primitive& primitiveData, const glm::mat4& transformMat);
	std::optional<Mesh> LoadMesh(const tinygltf::Model& modelData, const tinygltf::Mesh& meshData, const glm::mat4& transformMat);
	
public:
	
	Model(AssetManager* assetManager, const std::string& path)
		: Asset(assetManager, path),
          m_usage(GeometryUsage::STATIC) {}

	bool Load(const std::string& path, GeometryUsage usage);
	void Clear() override;

	const Info& GetInfo() const { return m_info; }

	const std::vector<Mesh>& GetMeshes() const { return m_meshes; }
	
	GeometryUsage GetUsage() const { return m_usage; }
};
