#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "world/nodes/sky/SkyboxNode.h"


namespace ogl {
struct GLBasicSkybox : NodeObserver<SkyboxNode, GLRendererBase> {
	GLTexture* cubemap;

	GLShader* shader;

	GLuint vao{};
	GLuint vbo{};

	void DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset) override {}
	GLBasicSkybox(SkyboxNode* node);
	~GLBasicSkybox() override;
};
} // namespace ogl
