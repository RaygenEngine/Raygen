#pragma once
#include "renderer/NodeObserver.h"
#include "renderer/renderers/opengl/GLRendererBase.h"
#include "asset/AssetPod.h"

namespace OpenGL
{
	class GLAsset : public RendererObject<GLRendererBase>
	{
	public:
		uri::Uri GetAssetManagerPodPath() const { return m_assetManagerPodPath; }

	protected:
		GLAsset(const uri::Uri& assocPath)
			: m_assetManagerPodPath(assocPath) {}
		virtual ~GLAsset() = default;

		uri::Uri m_assetManagerPodPath;
		
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
