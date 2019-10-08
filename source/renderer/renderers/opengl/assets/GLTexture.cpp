#include "pch.h"

#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/GLUtil.h"
#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"

namespace OpenGL
{
	GLTexture::~GLTexture()
	{
		glDeleteTextures(1, &id);
	}

	bool GLTexture::Load()
	{
		const auto textureData = podHandle.Lock();
		
		glGenTextures(1, &id);

		const GLint textureTarget = GetGLTextureTarget(textureData->target);
	
		glBindTexture(textureTarget, id);

		const auto minFiltering = GetGLFiltering(textureData->minFilter);

		glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GetGLFiltering(textureData->magFilter));
		glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, minFiltering);
		glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GetGLWrapping(textureData->wrapS));
		glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GetGLWrapping(textureData->wrapT));
		glTexParameteri(textureTarget, GL_TEXTURE_WRAP_R, GetGLWrapping(textureData->wrapR));

		const auto GetTypeAndInternalFormat = [](bool isHdr) -> std::pair<GLenum, GLint>
		{
			return isHdr ? std::make_pair(GL_FLOAT, GL_RGBA32F) : std::make_pair(GL_UNSIGNED_BYTE, GL_RGBA);
		};
		
		switch (textureData->target)
		{
		case TextureTarget::TEXTURE_2D:
		{
			const auto img = textureData->images.at(0);
			const auto typeAndInternalFormat = GetTypeAndInternalFormat(img->isHdr);
			glTexImage2D(GL_TEXTURE_2D, 0, typeAndInternalFormat.second, img->width, img->height, 0,
				GL_RGBA, typeAndInternalFormat.first, img->data);
			break;
		}
			
		case TextureTarget::TEXTURE_CUBEMAP:
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
		case TextureTarget::TEXTURE_1D:
		case TextureTarget::TEXTURE_3D:
		case TextureTarget::TEXTURE_ARRAY:
		case TextureTarget::TEXTURE_CUBEMAP_ARRAY:
		default:
			assert(false && "Texture format yet supported");
		}
		
		if (minFiltering == GL_NEAREST_MIPMAP_NEAREST ||
			minFiltering == GL_LINEAR_MIPMAP_NEAREST ||
			minFiltering == GL_NEAREST_MIPMAP_LINEAR ||
			minFiltering == GL_LINEAR_MIPMAP_LINEAR)
			glGenerateMipmap(textureTarget);

		return true;
	}
}
