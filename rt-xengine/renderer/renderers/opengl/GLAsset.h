#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"


namespace OpenGL
{
	
	class GLAsset : public RendererObject<GLRendererBase>
	{	
	protected:
		GLAsset(void* asset)
			: m_cacher(asset) {}
		virtual ~GLAsset() = default;

		void* m_cacher;
		bool m_isLoaded{ false };

		virtual bool Load() = 0;
		virtual void Unload() = 0;
	private:
		bool FriendLoad() { return Load(); }
		void FriendUnload() { Unload(); }

		friend class GLAssetManager;
	};

}
