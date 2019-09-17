#pragma once

#include "renderer/renderers/opengl/GLAsset.h"

#include "glad/glad.h"

namespace OpenGL
{
	struct GLTexture : GLAsset
	{
		GLuint id;

		GLTexture(const fs::path& assocPath)
			: GLAsset(assocPath),
			  id(0)
		{
		}
		virtual ~GLTexture();

		bool Load() override;
	};
}
