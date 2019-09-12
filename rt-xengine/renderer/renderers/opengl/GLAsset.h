#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"


namespace OpenGL
{
	
	class GLAsset : public RendererObject<GLRendererBase>
	{	
	protected:
		GLAsset(Asset* asset)
			: m_asset(asset) {}
		virtual ~GLAsset() = default;

		Asset* m_asset;
		bool m_isLoaded{ false };

		virtual bool Load() = 0;
		virtual void Unload() = 0;
	private:
		bool FriendLoad() { return Load(); }
		void FriendUnload() { Unload(); }

		friend class GLAssetManager;
	};

}
