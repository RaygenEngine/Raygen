#pragma once

#include "assets/Asset.h"

namespace Renderer::OpenGL
{
	class GLRendererBase;

	class GLAsset : public Assets::Asset
	{

	public:
		GLAsset(GLRendererBase* renderer, const std::string& name);
		virtual ~GLAsset() = default;

		GLRendererBase* GetGLRenderer() const;

		void ToString(std::ostream& os) const override { os << "asset-type: GLAsset, name: " << m_name; }
	};

}
