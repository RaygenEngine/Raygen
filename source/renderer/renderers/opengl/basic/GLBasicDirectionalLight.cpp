#include "pch.h"

#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"

namespace OpenGL
{
	GLBasicDirectionalLight::GLBasicDirectionalLight(DirectionalLightNode* node)
		: NodeObserver<DirectionalLightNode>(node)
	{
		// dir light depth TODO:
		glGenFramebuffers(1, &fbo);

		glGenTextures(1, &shadowMap);
		
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	GLBasicDirectionalLight::~GLBasicDirectionalLight()
	{
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &shadowMap);
	}
}
