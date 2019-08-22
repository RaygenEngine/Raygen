#ifndef GLASSET_H
#define GLASSET_H

#include "GLRendererBase.h"
#include "renderer/GPUAsset.h"


namespace Renderer::OpenGL
{

	class GLAsset : public GPUAsset
	{
		GLRendererBase* m_renderer;

	public:
		GLAsset(GLRendererBase* renderer);
		virtual ~GLAsset() = default;

	    GLRendererBase* GetRenderer() const { return m_renderer; }
	};

}

#endif // GLASSET_H
