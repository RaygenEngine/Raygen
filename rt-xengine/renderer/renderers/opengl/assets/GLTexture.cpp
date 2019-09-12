#include "pch.h"

#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/GLUtil.h"
#include "system/Engine.h"
#include "assets/AssetManager.h"

namespace OpenGL
{
	GLTexture::~GLTexture()
	{
		// TODO: handle bind-less
		glDeleteTextures(1, &m_glId);
	}

	bool GLTexture::Load()
	{
		if (!Engine::GetAssetManager()->Load(m_textureData))
			return false;

		glGenTextures(1, &m_glId);
		glBindTexture(GL_TEXTURE_2D, m_glId);

		auto minFiltering = GetGLFiltering(m_textureData->GetMinFilter());

		// If you don't use one of the filter values that include mipmaps (like GL_LINEAR_MIPMAP_LINEAR), your mipmaps will not be used in any way.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLFiltering(m_textureData->GetMagFilter()));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFiltering);

		GLenum type;
		GLint internalFormat;

		if (m_textureData->IsHdr())
		{
			type = GL_FLOAT;
			internalFormat = GL_RGBA32F;
		}
		else
		{
			type = GL_UNSIGNED_BYTE;
			internalFormat = GL_RGBA;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLWrapping(m_textureData->GetWrapS()));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLWrapping(m_textureData->GetWrapT()));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GetGLWrapping(m_textureData->GetWrapR()));
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_textureData->GetWidth(), m_textureData->GetHeight(), 0, GL_RGBA, type, m_textureData->GetData());


		if (minFiltering == GL_NEAREST_MIPMAP_NEAREST ||
			minFiltering == GL_LINEAR_MIPMAP_NEAREST ||
			minFiltering == GL_NEAREST_MIPMAP_LINEAR ||
			minFiltering == GL_LINEAR_MIPMAP_LINEAR)
			glGenerateMipmap(GL_TEXTURE_2D);

		// TODO if bindless?
		m_bindlessHandle = glGetTextureHandleARB(m_glId);
		glMakeTextureHandleResidentARB(m_bindlessHandle);

		return true;
	}

	void GLTexture::Unload()
	{
		// TODO: handle bind-less
		glDeleteTextures(1, &m_glId);
	}
}
