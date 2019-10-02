#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "world/nodes/sky/SkyCubeNode.h"


namespace OpenGL
{
	struct GLBasicSkybox : NodeObserver<SkyCubeNode, GLRendererBase>
	{
		GLTexture* cubemap;

		GLShader* shader;

		GLuint vao;
		GLuint vbo;
		
		GLBasicSkybox(SkyCubeNode* node);
		~GLBasicSkybox();
	};
}
