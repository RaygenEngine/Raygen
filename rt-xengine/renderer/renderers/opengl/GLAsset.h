#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"

namespace OpenGL
{
	
	class GLAsset : public RendererObject<GLRendererBase>
	{	
		bool m_loaded;

	public:
		GLAsset(const std::string& name)
			: m_loaded(false)
		{
			SetName(name);
		}

		virtual ~GLAsset() = default;

		bool IsLoaded() const { return m_loaded; }

		// call this in case of successful loading
		void MarkLoaded();

		void ToString(std::ostream& os) const override { os << "asset-type: GLAsset, name: " << m_name; }
	};

}
