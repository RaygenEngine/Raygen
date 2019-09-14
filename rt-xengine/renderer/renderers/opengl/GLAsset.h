#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "asset/AssetPod.h"

#include <filesystem>
namespace  fs = std::filesystem;

namespace OpenGL
{
	class GLAsset : public RendererObject<GLRendererBase>
	{
	public:
		fs::path GetAssetManagerPodPath() const { return m_assetManagerPodPath; }

	protected:
		GLAsset(const fs::path& assocPath)
			: m_assetManagerPodPath(assocPath) {}
		virtual ~GLAsset() = default;

		fs::path m_assetManagerPodPath;
		
		bool m_isLoaded{ false };

		virtual bool Load() = 0;
		virtual void Unload() = 0;
	private:
		bool FriendLoad() { return Load(); }
		void FriendUnload() { Unload(); }

		friend class GLAssetManager;
	};

}
