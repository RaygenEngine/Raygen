#include "pch/pch.h"

#include "renderer/renderers/opengl/assets/GLBindlessTexture.h"

namespace ogl {
GLBindlessTexture::~GLBindlessTexture()
{
	if (glIsTextureHandleResidentARB(handle)) {
		glMakeTextureHandleNonResidentARB(handle);
	}
}

void GLBindlessTexture::Load()
{
	handle = static_cast<GLuint>(glGetTextureHandleARB(id));
	glMakeTextureHandleResidentARB(handle);
}
} // namespace ogl
