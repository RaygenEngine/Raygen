#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"

#include "glad/glad.h"

namespace OpenGL
{
	struct GLBindlessTexture : GLTexture
	{
		GLuint handle{ 0 };

		GLBindlessTexture(const uri::Uri& assocPath)
			: GLTexture(assocPath)
		{
		}
		virtual ~GLBindlessTexture();

	protected:
		bool Load() override;
	};
}
