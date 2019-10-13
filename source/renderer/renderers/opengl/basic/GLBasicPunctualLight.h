#pragma once

#include "world/nodes/light/PunctualLightNode.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/assets/GLShader.h"


#include <glad/glad.h>


namespace ogl {
struct GLBasicPunctualLight : NodeObserver<PunctualLightNode, GLRendererBase> {
	GLuint fbo{};
	GLuint cubeShadowMap{};

	GLShader* depthMapShader{ nullptr };

	GLBasicPunctualLight(PunctualLightNode* node);
	~GLBasicPunctualLight() override;

	// render shadow map, then return the matrix needed to move from world to light space
	void RenderShadowMap(const std::vector<GLBasicGeometry*>& geometries);

	void DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset) override;
};
} // namespace ogl
