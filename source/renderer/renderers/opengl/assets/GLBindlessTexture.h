#pragma once

#include "renderer/renderers/opengl/assets/GLTexture.h"

#include <glad/glad.h>

namespace ogl {
// TODO: impl correctly (if ever needed)
struct GLBindlessTexture : GLTexture {
	GLuint handle{ 0 };

	GLBindlessTexture(PodHandle<TexturePod> handle)
		: GLTexture(handle)
	{
	}

	virtual ~GLBindlessTexture();

protected:
	void Load() override;
};
} // namespace ogl
