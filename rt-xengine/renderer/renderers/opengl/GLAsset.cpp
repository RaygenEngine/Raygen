#include "pch.h"

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

namespace Renderer::OpenGL
{
	GLAsset::GLAsset(GLAssetManager* glAssetManager, const std::string& name)
		: Asset(glAssetManager, name),
		  m_glAssetManager(glAssetManager)
	{
	}
}
