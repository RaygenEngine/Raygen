#include "pch.h"

#include "renderer/renderers/opengl/assets/GLCubeMap.h"

namespace OpenGL
{
	GLCubeMap::~GLCubeMap()
	{
		glDeleteTextures(1, &m_glId);
	}

	bool GLCubeMap::Load(CubeMapAsset* data, GLint wrapFlag, bool mipMapping)
	{
		glGenTextures(1, &m_glId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_glId);

		if (mipMapping)
		{
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		GLenum type;
		GLint internalFormat;
		if (data->IsHdr())
		{
			type = GL_FLOAT;
			internalFormat = GL_RGBA32F;
		}
		else
		{
			type = GL_UNSIGNED_BYTE;
			internalFormat = GL_RGBA;
		}

		// format is RGBA (stb)
		const GLenum format = GL_RGBA;

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapFlag);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapFlag);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapFlag);


		for (auto i = 0; i < CMF_COUNT; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat,
				data->GetWidth(), data->GetHeight(), 0, format, type, data->GetFace(static_cast<CubeMapFace>(i))->GetData());
		}

		return true;
	}
}
