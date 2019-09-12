#include "pch.h"

#include "renderer/renderers/opengl/assets/GLCubeMap.h"
#include "system/Engine.h"
#include "assets/AssetManager.h"
#include "renderer/renderers/opengl/GLUtil.h"

namespace OpenGL
{
	GLCubeMap::~GLCubeMap()
	{
		// TODO: handle bind-less
		glDeleteTextures(1, &m_glId);
	}

	bool GLCubeMap::Load()
	{
		if (!Engine::GetAssetManager()->Load(m_cubeMapData))
			return false;
		
		glGenTextures(1, &m_glId);
		glBindTexture(GL_TEXTURE_2D, m_glId);

		// everything matches the first texture
		const auto firstFaceText = m_cubeMapData->GetFace(CMF_RIGHT);
		const auto minFiltering = GetGLFiltering(firstFaceText->GetMinFilter());

		// If you don't use one of the filter values that include mipmaps (like GL_LINEAR_MIPMAP_LINEAR), your mipmaps will not be used in any way.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLFiltering(firstFaceText->GetMagFilter()));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFiltering);

		GLenum type;
		GLint internalFormat;

		if (firstFaceText->IsHdr())
		{
			type = GL_FLOAT;
			internalFormat = GL_RGBA32F;
		}
		else
		{
			type = GL_UNSIGNED_BYTE;
			internalFormat = GL_RGBA;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLWrapping(firstFaceText->GetWrapS()));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLWrapping(firstFaceText->GetWrapT()));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GetGLWrapping(firstFaceText->GetWrapR()));

		for (auto i = 0; i < CMF_COUNT; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat,
				firstFaceText->GetWidth(), firstFaceText->GetHeight(), 0, GL_RGBA, type, m_cubeMapData->GetFace(CubeMapFace(i))->GetData());
		}

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

	void GLCubeMap::Unload()
	{
		glDeleteTextures(1, &m_glId);
	}
}
