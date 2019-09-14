#include "pch.h"

#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/GLUtil.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"

namespace OpenGL
{
	GLTexture::~GLTexture()
	{
		// TODO: handle bind-less
		glDeleteTextures(1, &m_glId);
	}

	bool GLTexture::Load()
	{
		const auto textureData = Engine::GetAssetManager()->RequestFreshPod<TexturePod>(m_assetManagerPodPath);
		Engine::GetAssetManager()->RefreshPod(textureData->image);
		
		glGenTextures(1, &m_glId);
		glBindTexture(GL_TEXTURE_2D, m_glId);

		const auto minFiltering = GetGLFiltering(textureData->minFilter);

		// If you don't use one of the filter values that include mipmaps (like GL_LINEAR_MIPMAP_LINEAR), your mipmaps will not be used in any way.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLFiltering(textureData->magFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFiltering);

		GLenum type;
		GLint internalFormat;

		if (textureData->image->hdr)
		{
			type = GL_FLOAT;
			internalFormat = GL_RGBA32F;
		}
		else
		{
			type = GL_UNSIGNED_BYTE;
			internalFormat = GL_RGBA;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLWrapping(textureData->wrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLWrapping(textureData->wrapT));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GetGLWrapping(textureData->wrapR));
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, textureData->image->width, textureData->image->height, 0,
			GL_RGBA, type, textureData->image->data);


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
}
