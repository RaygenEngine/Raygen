#pragma once

#include "assets/Asset.h"

namespace OpenGL
{
	class GLAssetManager;
	
	class GLAsset : public Object
	{	
		GLAssetManager* m_glAssetManager;


		bool m_loaded;


	protected:
		std::string m_name;
		
	public:
		GLAsset(GLAssetManager* glAssetManager, const std::string& name)
			: m_glAssetManager(glAssetManager),
			  m_loaded(false) {}
		virtual ~GLAsset() = default;

		GLAssetManager* GetGLAssetManager() const { return m_glAssetManager; }

		bool IsLoaded() const { return m_loaded; }

		// call this in case of successful loading
		void MarkLoaded();

		void ToString(std::ostream& os) const override { os << "asset-type: GLAsset, name: " << m_name; }
	};

}
