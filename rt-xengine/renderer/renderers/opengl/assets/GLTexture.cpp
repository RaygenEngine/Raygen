#include "pch.h"

#include "renderer/renderers/opengl/assets/GLTexture.h"

namespace Renderer::OpenGL
{

	GLTexture::GLTexture(GLAssetManager* glAssetManager, const std::string& name)
		: GLAsset(glAssetManager, name), m_bindlessHandle(0),
		  m_glId(0), m_texCoordIndex(0)
	{
	}

	GLTexture::~GLTexture()
	{
		glDeleteTextures(1, &m_glId);
	}

	bool GLTexture::Load(Assets::Texture* data, GLint minFilter, GLint magFilter, GLint wrapS, GLint wrapT, GLint wrapR)
	{
		// TODO: where should i store this?>
		m_texCoordIndex = 0;

		glGenTextures(1, &m_glId);
		glBindTexture(GL_TEXTURE_2D, m_glId);

		// If you don't use one of the filter values that include mipmaps (like GL_LINEAR_MIPMAP_LINEAR), your mipmaps will not be used in any way.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);

		GLenum type;
		GLint internalFormat;
		switch (data->GetType())
		{

		case DynamicRange::HIGH:
			type = GL_FLOAT;
			internalFormat = GL_RGBA32F;
			break;

		case DynamicRange::LOW:
		default:
			type = GL_UNSIGNED_BYTE;
			internalFormat = GL_RGBA;
			break;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrapR);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, data->GetWidth(), data->GetHeight(), 0, GL_RGBA, type, data->GetData());


		if (minFilter == GL_NEAREST_MIPMAP_NEAREST ||
			minFilter == GL_LINEAR_MIPMAP_NEAREST ||
			minFilter == GL_NEAREST_MIPMAP_LINEAR ||
			minFilter == GL_LINEAR_MIPMAP_LINEAR)
			glGenerateMipmap(GL_TEXTURE_2D);

		// TODO if bindless?
		m_bindlessHandle = glGetTextureHandleARB(m_glId);
		glMakeTextureHandleResidentARB(m_bindlessHandle);
		
		return true;
	}
}
