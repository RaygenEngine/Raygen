#pragma once

#include "assets/DiskAssetPart.h"
#include "assets/model/Sampler.h"

namespace tinygltf
{
	class Model;
	struct Material;
}

namespace Assets
{
	// Note: assets of this class (Textures) are not cached directly as they are part of a cached Model anyway
	// glTF-based material (not all extensions included) (comments in this file -> https://github.com/KhronosGroup/glTF/tree/master/specification/2.0)
	class Material : public DiskAssetPart
	{
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
		Sampler m_baseColorTextureSampler;
		// The metallic and roughness properties are packed together in a single texture called metallicRoughnessTexture.
		// R-empty, G-roughness, B-metal, A-empty
		Sampler m_metallicRoughnessTextureSampler;
		// A tangent space normal map
		Sampler m_normalTextureSampler;
		// The occlusion map indicating areas of indirect lighting
		Sampler m_occlusionTextureSampler;
		// The emissive map controls the color and intensity of the light being emitted by the material.
		Sampler m_emissiveTextureSampler;

		// Factor values act as linear multipliers for the corresponding texture values.
		glm::vec4 m_baseColorFactor;
		glm::vec3 m_emissiveFactor;
		float m_metallicFactor;
		float m_roughnessFactor;

		// scaledNormal = normalize((<sampled normal texture value> * 2.0 - 1.0) * vec3(<normal scale>, <normal scale>, 1.0))
		float m_normalScale;
		// occludedColor = lerp(color, color * <sampled occlusion texture value>, <occlusion strength>)
		float m_occlusionStrength;

		// When alphaMode is set to MASK the alphaCutoff property specifies the cutoff threshold. If the alpha value is greater than or equal
		// to the alphaCutoff value then it is rendered as fully opaque, otherwise, it is rendered as fully transparent. alphaCutoff value is ignored for other modes.
		// The alpha value is defined in the baseColorTexture for metallic-roughness material model.
		AlphaMode m_alphaMode;
		float m_alphaCutoff;

		// The doubleSided property specifies whether the material is double sided. When this value is false, back-face culling is enabled. When this value is true,
		// back-face culling is disabled and double sided lighting is enabled. The back-face must have its normals reversed before the lighting equation is evaluated.
		bool m_doubleSided;

		
	public:
		Material(DiskAsset* pAsset, const std::string& name);

		void Load(const tinygltf::Model& modelData, const tinygltf::Material& materialData);
		
		const Sampler& GetBaseColorTextureSampler() const { return m_baseColorTextureSampler; }
		const Sampler& GetMetallicRoughnessTextureSampler() const { return m_metallicRoughnessTextureSampler; }
		const Sampler& GetNormalTextureSampler() const { return m_normalTextureSampler; }
		const Sampler& GetOcclusionTextureSampler() const { return m_occlusionTextureSampler; }
		const Sampler& GetEmissiveTextureSampler() const { return m_emissiveTextureSampler; }

		const glm::vec4& GetBaseColorFactor() const { return m_baseColorFactor; }
		const glm::vec3& GetEmissiveFactor() const { return m_emissiveFactor; }
		float GetMetallicFactor() const { return m_metallicFactor; }
		float GetRoughnessFactor() const { return m_roughnessFactor; }
		// scaledNormal = normalize((<sampled normal texture value> * 2.0 - 1.0) * vec3(<normal scale>, <normal scale>, 1.0))
		float GetNormalScale() const { return m_normalScale; }
		// occludedColor = lerp(color, color * <sampled occlusion texture value>, <occlusion strength>)
		float GetOcclusionStrength() const { return m_occlusionStrength; }
		// When alphaMode is set to MASK the alphaCutoff property specifies the cutoff threshold. If the alpha value is greater than or equal
		// to the alphaCutoff value then it is rendered as fully opaque, otherwise, it is rendered as fully transparent. alphaCutoff value is ignored for other modes.
		// The alpha value is defined in the baseColorTexture for metallic-roughness material model.
		AlphaMode GetAlphaMode() const { return m_alphaMode; }
		float GetAlphaCutoff() const { return m_alphaCutoff; }
		// The doubleSided property specifies whether the material is double sided. When this value is false, back-face culling is enabled. When this value is true,
		// back-face culling is disabled and double sided lighting is enabled. The back-face must have its normals reversed before the lighting equation is evaluated.
		bool IsDoubleSided() const { return m_doubleSided; }

		void ToString(std::ostream& os) const override { os << "asset-type: Material, name: " << m_name; }
	};
}
