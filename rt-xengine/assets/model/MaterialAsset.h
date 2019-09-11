#pragma once

#include "assets/Asset.h"
#include "assets/texture/TextureAsset.h"

// Note: assets of this class (Textures) are not cached directly as they are part of a cached ModelAsset anyway
// glTF-based material (not all extensions included) (comments in this file -> https://github.com/KhronosGroup/glTF/tree/master/specification/2.0)
class MaterialAsset : public Asset
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
	TextureAsset* m_baseColorTexture;
	int32 m_baseColorTexCoordIndex;
	// The metallic and roughness properties are packed together in a single texture called metallicRoughnessTexture.
	// R-occlusion, G-roughness, B-metal, A-empty
	TextureAsset* m_occlusionMetallicRoughnessTexture;
	int32 m_occlusionMetallicRoughnessTexCoordIndex;
	// A tangent space normal map
	TextureAsset* m_normalTexture;
	int32 m_normalTexCoordIndex;
	// The emissive map controls the color and intensity of the light being emitted by the material.
	TextureAsset* m_emissiveTexture;
	int32 m_emissiveTexCoordIndex;

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
	
	MaterialAsset(const fs::path& path)
		: Asset(path),
		  m_baseColorTexture(nullptr),
	      m_baseColorTexCoordIndex(0),
		  m_occlusionMetallicRoughnessTexture(nullptr),
	      m_occlusionMetallicRoughnessTexCoordIndex(0),
		  m_normalTexture(nullptr),
	      m_normalTexCoordIndex(0),
		  m_emissiveTexture(nullptr),
	      m_emissiveTexCoordIndex(0),
		  m_baseColorFactor(1.f, 1.f, 1.f, 1.f),
		  m_emissiveFactor(0.f, 0.f, 0.f),
		  m_metallicFactor(1.f),
		  m_roughnessFactor(1.f),
		  m_normalScale(1.f),
		  m_occlusionStrength(1.f),
		  m_alphaMode(AM_OPAQUE),
		  m_alphaCutoff(0.5f),
		  m_doubleSided(false)
	{
	}


	TextureAsset* GetBaseColorTexture() const { return m_baseColorTexture; }
	TextureAsset* GetOcclusionMetallicRoughnessTexture() const { return m_occlusionMetallicRoughnessTexture; }
	TextureAsset* GetNormalTexture() const { return m_normalTexture; }
	TextureAsset* GetEmissiveTexture() const { return m_emissiveTexture; }

	int32 GetBaseColorTexCoordIndex() const { return m_baseColorTexCoordIndex; }
	int32 GetOcclusionMetallicRoughnessTexCoordIndex() const { return m_occlusionMetallicRoughnessTexCoordIndex; }
	int32 GetNormalTexCoordIndex() const { return m_normalTexCoordIndex; }
	int32 GetEmissiveTexCoordIndex() const { return m_emissiveTexCoordIndex; }

	glm::vec4 GetBaseColorFactor() const { return m_baseColorFactor; }
	glm::vec3 GetEmissiveFactor() const { return m_baseColorFactor; }
	float GetMetallicFactor() const { return m_metallicFactor; }
	float GetRoughnessFactor() const { return m_roughnessFactor; }
	float GetNormalScale() const { return m_normalScale; }
	float GetOcclusionStrength() const { return m_occlusionStrength; }
	AlphaMode GetAlphaMode() const { return m_alphaMode; }
	float GetAlphaCutoff() const { return m_alphaCutoff; }
	bool IsDoubleSided() const { return m_doubleSided; }

protected:
	bool Load() override;
	void Unload() override;
};
