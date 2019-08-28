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
		
	public:

		GLMaterial(GLRendererBase* renderer, const std::string& name);
		~GLMaterial() = default;
		
		bool Load(Assets::Material* data);

		GLTexture* GetBaseColorTexture() const { return m_baseColorTexture.get(); }
		GLTexture* GetMetallicRoughnessTexture() const { return m_metallicRoughnessTexture.get(); }
		GLTexture* GetNormalTexture() const { return m_normalTexture.get(); }
		GLTexture* GetOcclusionTexture() const { return m_occlusionTexture.get(); }
		GLTexture* GetEmissiveTexture() const { return m_emissiveTexture.get(); }

		void ToString(std::ostream& os) const override { os << "asset-type: GLMaterial, name: " << m_name; }
	};

}
