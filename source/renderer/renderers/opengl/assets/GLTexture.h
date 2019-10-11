#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "asset/pods/TexturePod.h"

#include <glad/glad.h>

namespace ogl {
struct GLTexture : GLAsset<TexturePod> {
	GLuint id{ 0 };

	GLTexture(PodHandle<TexturePod> handle)
		: GLAsset(handle)
	{
	}

	~GLTexture() override;

	bool Load() override;
};
} // namespace ogl
