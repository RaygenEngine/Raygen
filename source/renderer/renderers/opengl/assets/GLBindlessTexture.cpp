#include "pch/pch.h"

#include "renderer/renderers/opengl/assets/GLBindlessTexture.h"

namespace ogl {
GLBindlessTexture::~GLBindlessTexture()
{
	if (glIsTextureHandleResidentARB(handle)) {
		glMakeTextureHandleNonResidentARB(handle);
	}
}

bool GLBindlessTexture::Load()
{
	handle = static_cast<GLuint>(glGetTextureHandleARB(id));
	glMakeTextureHandleResidentARB(handle);

	return true;
}
} // namespace ogl
