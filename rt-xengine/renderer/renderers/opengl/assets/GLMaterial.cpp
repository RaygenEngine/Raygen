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
		/*SetIdentificationFromAssociatedDiskAssetIdentification(data->GetLabel());

		m_textSurfaceAlbedo = GetRenderer()->RequestGLTexture(data->GetMapSurfaceAlbedo());
		
		m_textSurfaceEmission = GetRenderer()->RequestGLTexture(data->GetMapSurfaceEmission());

		m_textSurfaceSpecularParameters = GetRenderer()->RequestGLTexture(data->GetMapSurfaceSpecularParameters());

		m_textSurfaceBump = GetRenderer()->RequestGLTexture(data->GetMapSurfaceBump());*/

		return true;
	}
}
