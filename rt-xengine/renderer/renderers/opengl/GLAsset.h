#pragma once

#include "assets/Asset.h"

namespace OpenGL
{
	class GLAssetManager;
	
	class GLAsset : public Object
	{	
		GLAssetManager* m_glAssetManager;

	protected:
		std::string m_name;
		
	public:
		GLAsset(GLAssetManager* glAssetManager, const std::string& name)
			: m_glAssetManager(glAssetManager) {}
		virtual ~GLAsset() = default;

		GLAssetManager* GetGLAssetManager() const { return m_glAssetManager; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLAsset, name: " << m_name; }
	};

}
