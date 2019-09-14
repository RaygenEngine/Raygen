#pragma once
#include "renderer/NodeObserver.h"
#include "renderer/renderers/opengl/GLRendererBase.h"

namespace OpenGL
{
	
	class GLAsset : public RendererObject<GLRendererBase>
	{	
	protected:
		GLAsset() {}
		virtual ~GLAsset() = default;

		bool m_isLoaded{ false };

		virtual bool Load() = 0;

	private:
		bool FriendLoad() 
		{ 
			m_isLoaded = Load(); 
			return m_isLoaded;
		}

		friend class GLAssetManager;
	};

}
