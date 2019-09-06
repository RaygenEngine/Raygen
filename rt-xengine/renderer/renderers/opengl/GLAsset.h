#pragma once

#include "assets/Asset.h"
#include "renderer/renderers/opengl/GLRendererBase.h"

namespace OpenGL
{
	
	class GLAsset : public RendererObject<GLRendererBase>
	{	
		bool m_loaded;

	protected:
		std::string m_name;
		
	public:
		GLAsset(GLAssetManager* glAssetManager, const std::string& name)
			  : m_loaded(false) {}
		virtual ~GLAsset() = default;

		bool IsLoaded() const { return m_loaded; }

		// call this in case of successful loading
		void MarkLoaded();

		void ToString(std::ostream& os) const override { os << "asset-type: GLAsset, name: " << m_name; }
	};

}
