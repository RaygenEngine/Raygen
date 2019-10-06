#include "pch.h"

#include "renderer/renderers/opengl/assets/GLBindlessTexture.h"

namespace OpenGL
{
	GLBindlessTexture::~GLBindlessTexture()
	{
		if (glIsTextureHandleResidentARB(handle))
			glMakeTextureHandleNonResidentARB(handle);
	}

	bool GLBindlessTexture::Load()
	{
		handle = static_cast<GLuint>(glGetTextureHandleARB(id));
		glMakeTextureHandleResidentARB(handle);
		
		return true;
	}
}
