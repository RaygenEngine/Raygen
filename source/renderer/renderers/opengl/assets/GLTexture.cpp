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
		glDeleteTextures(1, &m_id);
	}

	bool GLTexture::Load()
	{
		const auto textureData = AssetManager::GetOrCreate<TexturePod>(m_assetManagerPodPath);
		
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);

		//if(textureData->type)
		
		const auto minFiltering = GetGLFiltering(textureData->minFilter);

		// If you don't use one of the filter values that include mipmaps (like GL_LINEAR_MIPMAP_LINEAR), your mipmaps will not be used in any way.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLFiltering(textureData->magFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFiltering);


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLWrapping(textureData->wrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLWrapping(textureData->wrapT));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GetGLWrapping(textureData->wrapR));


		auto GetTypeAndInternalFormat = [](bool isHdr) -> std::pair<GLenum, GLint>
		{
			return isHdr ? std::make_pair(GL_FLOAT, GL_RGBA32F) : std::make_pair(GL_UNSIGNED_BYTE, GL_RGBA);
		};
		
		switch (textureData->type)
		{
		case TextureType::TEXTURE_2D:
		{
			const auto img = textureData->images.at(0);
			const auto typeAndInternalFormat = GetTypeAndInternalFormat(img->isHdr);
			glTexImage2D(GL_TEXTURE_2D, 0, typeAndInternalFormat.second, img->width, img->height, 0,
				GL_RGBA, typeAndInternalFormat.first, img->data);
			break;
		}
			
		case TextureType::TEXTURE_CUBEMAP:
		{
			for (auto i = 0; i < CMF_COUNT; ++i)
			{
				const auto img = textureData->images.at(i);
				const auto typeAndInternalFormat = GetTypeAndInternalFormat(img->isHdr);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, typeAndInternalFormat.second,
					img->width, img->height, 0, GL_RGBA, typeAndInternalFormat.first, img->data);
			}
			break;
		}
		case TextureType::TEXTURE_1D:
		case TextureType::TEXTURE_3D:
		case TextureType::TEXTURE_ARRAY:
		case TextureType::TEXTURE_CUBEMAP_ARRAY:
		default:
			assert(false && "Not yet supported");
		}
		
		if (minFiltering == GL_NEAREST_MIPMAP_NEAREST ||
			minFiltering == GL_LINEAR_MIPMAP_NEAREST ||
			minFiltering == GL_NEAREST_MIPMAP_LINEAR ||
			minFiltering == GL_LINEAR_MIPMAP_LINEAR)
			glGenerateMipmap(GL_TEXTURE_2D);

		// TODO if bindless, static flag?
		m_bindlessId = glGetTextureHandleARB(m_id);
		glMakeTextureHandleResidentARB(m_bindlessId);

		return true;
	}
}
