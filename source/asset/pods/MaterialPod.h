#pragma once

#include "system/reflection/Reflector.h"
#include "asset/pods/TexturePod.h"

// Note: assets of this class (Textures) are not cached directly as they are part of a cached ModelAsset anyway
// glTF-based material (not all extensions included) (comments in this file -> https://github.com/KhronosGroup/glTF/tree/master/specification/2.0)
struct MaterialPod : AssetPod
{
	STATIC_REFLECTOR(MaterialPod)
	{
		S_REFLECT_VAR(baseColorTexture);
		S_REFLECT_VAR(occlusionMetallicRoughnessTexture);
		S_REFLECT_VAR(normalTexture);
		S_REFLECT_VAR(emissiveTexture);

		S_REFLECT_VAR(metallicFactor);
		S_REFLECT_VAR(roughnessFactor);
		S_REFLECT_VAR(normalScale);
		S_REFLECT_VAR(occlusionStrength);
		S_REFLECT_VAR(alphaCutoff);
		S_REFLECT_VAR(doubleSided);
	}
	static bool Load(MaterialPod* pod, const fs::path& path);

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
	PodHandle<TexturePod> baseColorTexture;
	int32 baseColorTexCoordIndex{ 0 };

	// The metallic and roughness properties are packed together in a single texture called metallicRoughnessTexture.
	// R-occlusion, G-roughness, B-metal, A-empty
	PodHandle<TexturePod> occlusionMetallicRoughnessTexture;
	int32 occlusionMetallicRoughnessTexCoordIndex{ 0 };
	
	// A tangent space normal map
	PodHandle<TexturePod> normalTexture;
	int32 normalTexCoordIndex{ 0 };
	
	// The emissive map controls the color and intensity of the light being emitted by the material.
	PodHandle<TexturePod> emissiveTexture;
	int32 emissiveTexCoordIndex{ 0 };

	// Factor values act as linear multipliers for the corresponding texture values.
	glm::vec4 baseColorFactor{ 1.f, 1.f, 1.f, 1.f };
	glm::vec3 emissiveFactor{ 0.f, 0.f, 0.f };
	float metallicFactor{ 1.f };
	float roughnessFactor{ 1.f };

	// scaledNormal = normalize((<sampled normal texture value> * 2.0 - 1.0) * vec3(<normal scale>, <normal scale>, 1.0))
	float normalScale{ 1.f };

	// occludedColor = lerp(color, color * <sampled occlusion texture value>, <occlusion strength>)
	float occlusionStrength{ 1.f };

	// When alphaMode is set to MASK the alphaCutoff property specifies the cutoff threshold. If the alpha value is greater than or equal
	// to the alphaCutoff value then it is rendered as fully opaque, otherwise, it is rendered as fully transparent. alphaCutoff value is ignored for other modes.
	// The alpha value is defined in the baseColorTexture for metallic-roughness material model.
	AlphaMode alphaMode{ AM_OPAQUE };
	float alphaCutoff{ 0.5f };

	// The doubleSided property specifies whether the material is double sided. When this value is false, back-face culling is enabled. When this value is true,
	// back-face culling is disabled and double sided lighting is enabled. The back-face must have its normals reversed before the lighting equation is evaluated.
	bool doubleSided{ false };
};
