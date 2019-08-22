#include "pch.h"
#include "GLMaterial.h"

namespace Renderer::OpenGL
{
	GLMaterial::GLMaterial(GLRendererBase* renderer)
		: GLAsset(renderer)
	{
	}

	bool GLMaterial::Load(Assets::XMaterial* data)
	{
		SetIdentificationFromAssociatedDiskAssetIdentification(data->GetLabel());

		m_textSurfaceAlbedo = GetRenderer()->RequestGLTexture(data->GetMapSurfaceAlbedo());
		
		m_textSurfaceEmission = GetRenderer()->RequestGLTexture(data->GetMapSurfaceEmission());

		m_textSurfaceSpecularParameters = GetRenderer()->RequestGLTexture(data->GetMapSurfaceSpecularParameters());

		m_textSurfaceBump = GetRenderer()->RequestGLTexture(data->GetMapSurfaceBump());

		return true;
	}
}
