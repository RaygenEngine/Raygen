#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "assets/texture/TextureAsset.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	class GLTexture : public GLAsset
	{
		// bindless
		GLuint64 m_bindlessHandle;
		GLuint m_glId;

	public:
		GLTexture(const std::string& name)
			: GLAsset(name),
		      m_bindlessHandle(0),
			  m_glId(0),
		      m_texCoordIndex(0) {}
		~GLTexture();

		int32 m_texCoordIndex;
		
		bool Load(TextureAsset* data, GLint minFilter, GLint magFilter, GLint wrapS, GLint wrapT, GLint wrapR);

		GLuint GetGLId() const { return m_glId; }
		GLuint64 GetGLBindlessHandle() const { return m_bindlessHandle; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLTexture, name: " << m_name; }
	};
}
