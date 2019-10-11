#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"

#include <glad/glad.h>

namespace OpenGL {
// TODO: impl correctly (when needed)
struct GLBindlessTexture : GLTexture {
	GLuint handle{ 0 };

	GLBindlessTexture(PodHandle<TexturePod> handle)
		: GLTexture(handle)
	{
	}

	virtual ~GLBindlessTexture();

protected:
	bool Load() override;
};
} // namespace OpenGL
