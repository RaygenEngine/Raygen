#pragma once

#include "world/nodes/light/SpotLightNode.h"
#include "renderer/renderers/opengl/GLRendererBase.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"

#include "glad/glad.h"


namespace OpenGL
{
	struct GLBasicSpotLight : NodeObserver<SpotLightNode, GLRendererBase>
	{
		GLuint fbo;
		GLuint shadowMap;

		GLShader* shader;

		glm::mat4 lightSpaceMatrix;
		
		GLBasicSpotLight(SpotLightNode* node);
		~GLBasicSpotLight();

		// render shadow map, then return the matrix needed to move from world to light space
		void RenderShadowMap(const std::vector<std::unique_ptr<GLBasicGeometry>>& geometries);
	};
}