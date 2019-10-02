#pragma once

#include "world/nodes/light/DirectionalLightNode.h"
#include "renderer/renderers/opengl/GLRendererBase.h"
#include "renderer/renderers/opengl/assets/GLShader.h"

#include "glad/glad.h"

namespace OpenGL
{
	struct GLBasicDirectionalLight : NodeObserver<DirectionalLightNode, GLRendererBase>
	{
		int32 width = 2048;
		int32 height = 2048;
		
		GLuint fbo;
		GLuint shadowMap;

		GLShader* shader;
		
		GLBasicDirectionalLight(DirectionalLightNode* node);
		~GLBasicDirectionalLight();
	};
}
