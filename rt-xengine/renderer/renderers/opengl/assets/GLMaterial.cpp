#include "pch.h"

#include "renderer/renderers/opengl/assets/GLMaterial.h"

namespace Renderer::OpenGL
{
	GLMaterial::GLMaterial(GLRendererBase* renderer, const std::string& name)
		: GLAsset(renderer, name)
	{
	}

	bool GLMaterial::Load(Assets::Material* data)
	{
		//m_baseColorTexture = GetRenderer()->RequestGLTexture(data->GetMapSurfaceAlbedo());
		//m_metallicRoughnessTexture = GetRenderer()->RequestGLTexture(data->GetMapSurfaceEmission());

		//m_normalTexture;
		//m_occlusionTexture;
		//m_emissiveTexture;
	
		return true;
	}
}
