#include "pch.h"
#include "GLTexture.h"

namespace Renderer::OpenGL
{

	GLTexture::GLTexture(GLRendererBase* renderer)
		: GLAsset(renderer),
		  m_textureId(0)
	{
	}

	GLTexture::~GLTexture()
	{
		glDeleteTextures(1, &m_textureId);
	}

	bool GLTexture::Load(Assets::Texture* data, GLint wrapFlag, bool mipMapping)
	{
		SetIdentificationFromAssociatedDiskAssetIdentification(data->GetLabel());

		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_2D, m_textureId);

		if (mipMapping)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		GLenum type;
		GLint internalFormat;
		switch (data->GetType())
		{

		case Assets::DR_HIGH:
			type = GL_FLOAT;
			internalFormat = GL_RGBA32F;
			break;

		case Assets::DR_LOW:
		default:
			type = GL_UNSIGNED_BYTE;
			internalFormat = GL_RGBA;
			break;
		}

		// format is RGBA (stb)
		const GLenum format = GL_RGBA;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFlag);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFlag);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, data->GetWidth(), data->GetHeight(), 0, format, type, data->GetData());

		return true;
	}
}
