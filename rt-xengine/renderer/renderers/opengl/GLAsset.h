#pragma once

#include "assets/Asset.h"

namespace Renderer::OpenGL
{
	class GLRendererBase;

	class GLAsset : public Assets::Asset
	{
		GLRendererBase* m_renderer;

	public:
		GLAsset(GLRendererBase* renderer, const std::string& name);
		virtual ~GLAsset() = default;

	    GLRendererBase* GetGLRenderer() const { return m_renderer; }
	};

}
