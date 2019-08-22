#include "pch.h"
#include "GLAsset.h"

namespace Renderer::OpenGL
{
	GLAsset::GLAsset(GLRendererBase* renderer)
		: GPUAsset(renderer),
		  m_renderer(renderer)
	{
	}
}
