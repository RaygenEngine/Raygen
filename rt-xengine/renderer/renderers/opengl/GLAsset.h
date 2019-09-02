#pragma once

#include "assets/Asset.h"

namespace Renderer::OpenGL
{
	class GLAssetManager;
	
	class GLAsset : public Assets::Asset
	{
		GLAssetManager* m_glAssetManager;
		
	public:
		GLAsset(GLAssetManager* glAssetManager, const std::string& name);
		virtual ~GLAsset() = default;

		GLAssetManager* GetGLAssetManager() const { return m_glAssetManager; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLAsset, name: " << m_name; }
	};

}
