#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "assets/texture/CubeMap.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	// TODO: This asset has not been tested in a renderer, if it doesn't work please implement it correctly, if it works, remove this comment
	class GLCubeMap : public GLAsset
	{
		GLuint m_glId;

	public:
		GLCubeMap(const std::string& name)
			: GLAsset(name),
			  m_glId(0) {}
		~GLCubeMap();

		bool Load(CubeMap* data, GLint wrapFlag, bool mipMapping);

		GLuint GetGLId() const { return m_glId; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLCubeTexture, name: " << m_name; }
	};
}
