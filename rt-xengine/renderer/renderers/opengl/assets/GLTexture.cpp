#include "pch.h"

#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/GLUtil.h"

namespace Renderer::OpenGL
{

	GLTexture::GLTexture(GLRendererBase* renderer, const std::string& name)
		: GLAsset(renderer, name),
		  m_textureId(0)
	{
	}

	GLTexture::~GLTexture()
	{
		glDeleteTextures(1, &m_textureId);
	}

	bool GLTexture::Load(Assets::Texture* data, GLint minFilter, GLint magFilter, GLint wrapS, GLint wrapT, GLint wrapR)
	{
		// TODO: where should i store this?>
		m_texCoordIndex = 0;

		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_2D, m_textureId);

		// If you don't use one of the filter values that include mipmaps (like GL_LINEAR_MIPMAP_LINEAR), your mipmaps will not be used in any way.
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);

		GLenum type;
		GLint internalFormat;
		switch (data->GetType())
		{

		case DR_HIGH:
			type = GL_FLOAT;
			internalFormat = GL_RGBA32F;
			break;

		case DR_LOW:
		default:
			type = GL_UNSIGNED_BYTE;
			internalFormat = GL_RGBA;
			break;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrapR);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, data->GetWidth(), data->GetHeight(), 0, GL_RGBA, type, data->GetData());
	
		return true;
	}
}
