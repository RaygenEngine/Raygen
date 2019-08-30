#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "assets/texture/Texture.h"

#include "GLAD/glad.h"

namespace Renderer::OpenGL
{
	class GLTexture : public GLAsset
	{
		GLuint m_textureId;

	public:
		GLTexture(GLRendererBase* renderer, const std::string& name);
		~GLTexture();

		int32 m_texCoordIndex;
		
		bool Load(Assets::Texture* data, GLint minFilter, GLint magFilter, GLint wrapS, GLint wrapT, GLint wrapR);

		GLuint GetGLHandle() const { return m_textureId; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLTexture, name: " << m_name; }
	};
}
