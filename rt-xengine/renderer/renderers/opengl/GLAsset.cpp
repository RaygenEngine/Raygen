#include "pch.h"

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/GLRendererBase.h"

namespace Renderer::OpenGL
{
	GLAsset::GLAsset(GLRendererBase* renderer, const std::string& name)
		: Asset(renderer, name)
	{
	}

	GLRendererBase* GLAsset::GetGLRenderer() const
	{
		return dynamic_cast<GLRendererBase*>(GetRenderer());
	}
}
