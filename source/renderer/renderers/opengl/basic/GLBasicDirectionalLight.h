#pragma once

#include "world/nodes/light/DirectionalLightNode.h"
#include "renderer/renderers/opengl/GLRendererBase.h"

#include "glad/glad.h"

namespace OpenGL
{
	struct GLBasicDirectionalLight : NodeObserver<DirectionalLightNode>
	{
		int32 width = 1024;
		int32 height = 1024;
		GLuint fbo;
		GLuint shadowMap;
		
		GLBasicDirectionalLight(DirectionalLightNode* node);
		~GLBasicDirectionalLight();
	};
}
