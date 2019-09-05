#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "assets/model/Material.h"

namespace Renderer::OpenGL
{
	class GLMaterial : public GLAsset 
	{
		// RGB: Albedo A: Opacity
		std::shared_ptr<GLTexture> m_baseColorTexture;
		// R: empty, G: Roughness, B: Metal, A: empty
		std::shared_ptr<GLTexture> m_metallicRoughnessTexture;
		std::shared_ptr<GLTexture> m_normalTexture;
		std::shared_ptr<GLTexture> m_occlusionTexture;
		std::shared_ptr<GLTexture> m_emissiveTexture;

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

		GLMaterial(GLAssetManager* glAssetManager, const std::string& name);

		GLMaterial(const GLMaterial& other) = delete;

		GLMaterial(GLMaterial&& other) = delete;

		GLMaterial& operator=(const GLMaterial& other) = delete;

		GLMaterial& operator=(GLMaterial&& other) = delete;

		bool Load(const Assets::Material& data);

		GLTexture* GetBaseColorTexture() const { return m_baseColorTexture.get(); }
		GLTexture* GetMetallicRoughnessTexture() const { return m_metallicRoughnessTexture.get(); }
		GLTexture* GetNormalTexture() const { return m_normalTexture.get(); }
		GLTexture* GetOcclusionTexture() const { return m_occlusionTexture.get(); }
		GLTexture* GetEmissiveTexture() const { return m_emissiveTexture.get(); }

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

		void ToString(std::ostream& os) const override { os << "asset-type: GLMaterial, name: " << m_name; }
	};

}
