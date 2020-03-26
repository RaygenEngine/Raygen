#pragma once
#include "assets/pods/SamplerPod.h"
#include "reflection/GenMacros.h"

// This material is based on the glTF standard for materials (not all extensions included)
// see -> https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#materials)
struct MaterialPod : AssetPod {

	enum AlphaMode : int32
	{
		// The rendered output is fully opaque and any alpha value is ignored.
		Opaque,
		// The rendered output is either fully opaque or fully transparent depending on the alpha value and the
		// specified alpha cutoff value. This mode is used to simulate geometry such as tree leaves or wire fences.
		Mask,
		// The rendered output is combined with the background using the normal painting operation (i.e. the Porter and
		// Duff over operator). This mode is used to simulate geometry such as guaze cloth or animal fur.
		Blend
	};

	REFLECTED_POD(MaterialPod)
	{
		REFLECT_ICON(FA_SWATCHBOOK);

		REFLECT_VAR(baseColorUvIndex, PropertyFlags::Hidden);
		REFLECT_VAR(metallicRoughnessUvIndex, PropertyFlags::Hidden);
		REFLECT_VAR(occlusionUvIndex, PropertyFlags::Hidden);
		REFLECT_VAR(normalUvIndex, PropertyFlags::Hidden);
		REFLECT_VAR(emissiveUvIndex, PropertyFlags::Hidden);

		REFLECT_VAR(baseColorFactor, PropertyFlags::Color);
		REFLECT_VAR(emissiveFactor, PropertyFlags::Color);

		REFLECT_VAR(metallicFactor);
		REFLECT_VAR(roughnessFactor);
		REFLECT_VAR(normalScale);
		REFLECT_VAR(occlusionStrength);
		REFLECT_VAR(alphaMode);
		REFLECT_VAR(alphaCutoff);
		REFLECT_VAR(doubleSided);
		REFLECT_VAR(castsShadows);

		REFLECT_VAR(baseColorImage);
		REFLECT_VAR(metallicRoughnessImage);
		REFLECT_VAR(occlusionImage);
		REFLECT_VAR(normalImage);
		REFLECT_VAR(emissiveImage);

		REFLECT_VAR(baseColorSampler, PropertyFlags::Advanced);
		REFLECT_VAR(metallicRoughnessSampler, PropertyFlags::Advanced);
		REFLECT_VAR(occlusionSampler, PropertyFlags::Advanced);
		REFLECT_VAR(normalSampler, PropertyFlags::Advanced);
		REFLECT_VAR(emissiveSampler, PropertyFlags::Advanced);
	}

	// The value for each property(baseColor, metallic, roughness) can be defined using factors or textures.

	// If a texture is not given, all respective texture components within this material model are assumed to have a
	// value of 1.0. If both factors and textures are present the factor value acts as a linear multiplier for the
	// corresponding texture values. The baseColorTexture uses the sRGB transfer function and must be converted to
	// linear space before it is used for any computations.

	// The base color has two different interpretations depending on the value of metalness.
	// When the material is a metal, the base color is the specific measured reflectance value at normal incidence (F0).
	// For a non-metal the base color represents the reflected diffuse color of the material.
	// In this model it is not possible to specify a F0 value for non-metals, and a linear value of 4% (0.04) is used.
	// The baseColorTexture uses the sRGB transfer function and must be converted to linear space before it is used for
	// any computations. R-red, G-green, B-blue, A-alpha
	PodHandle<ImagePod> baseColorImage;
	PodHandle<SamplerPod> baseColorSampler;
	int32 baseColorUvIndex{ 0 };

	// The metallic and roughness properties are packed together in a single texture called metallicRoughnessTexture.
	// R-occlusion, G-roughness, B-metal, A-empty
	PodHandle<ImagePod> metallicRoughnessImage;
	PodHandle<SamplerPod> metallicRoughnessSampler;
	int32 metallicRoughnessUvIndex{ 0 };

	// The metallic and roughness properties are packed together in a single texture called metallicRoughnessTexture.
	// R-occlusion, G-occlusion, B-occlusion, A-empty
	// Note: may point to an occlusionMetallicRoughness packed image where the R channel is equal to occlusion
	// use the R channel ALWAYS
	PodHandle<ImagePod> occlusionImage;
	PodHandle<SamplerPod> occlusionSampler;
	int32 occlusionUvIndex{ 0 };

	// A tangent space normal map
	PodHandle<ImagePod> normalImage{ GetDefaultNormalImagePodUid() };
	PodHandle<SamplerPod> normalSampler;
	int32 normalUvIndex{ 0 };

	// The emissive map controls the color and intensity of the light being emitted by the material.
	PodHandle<ImagePod> emissiveImage;
	PodHandle<SamplerPod> emissiveSampler;
	int32 emissiveUvIndex{ 0 };

	// Factor values act as linear multipliers for the corresponding texture values.
	glm::vec4 baseColorFactor{ 1.f, 1.f, 1.f, 1.f };
	glm::vec3 emissiveFactor{ 0.f, 0.f, 0.f };
	float metallicFactor{ 1.f };
	float roughnessFactor{ 1.f };

	// scaledNormal = normalize((<sampled normal texture value> * 2.0 - 1.0) * vec3(<normal scale>, <normal
	// scale>, 1.0))
	float normalScale{ 1.f };

	// occludedColor = lerp(color, color * <sampled occlusion texture value>, <occlusion strength>)
	float occlusionStrength{ 1.f };

	// When alphaMode is set to Mask the alphaCutoff property specifies the cutoff threshold. If the alpha value is
	// greater than or equal to the alphaCutoff value then it is rendered as fully opaque, otherwise, it is rendered as
	// fully transparent. alphaCutoff value is ignored for other modes. The alpha value is defined in the
	// baseColorTexture for metallic-roughness material model.
	AlphaMode alphaMode{ Opaque };
	float alphaCutoff{ 0.5f };

	// The doubleSided property specifies whether the material is double sided. When this value is false, back-face
	// culling is enabled. When this value is true, back-face culling is disabled and double sided lighting is enabled.
	// The back-face must have its normals reversed before the lighting equation is evaluated.
	bool doubleSided{ false };

	bool castsShadows{ true };
};