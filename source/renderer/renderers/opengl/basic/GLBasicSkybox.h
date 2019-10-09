#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "world/nodes/sky/SkyCubeNode.h"


namespace OpenGL {
struct GLBasicSkybox : NodeObserver<SkyCubeNode, GLRendererBase> {
	GLTexture* cubemap;

	GLShader* shader;

	GLuint vao;
	GLuint vbo;

	virtual void DirtyNodeUpdate(std::bitset<64> nodeDirtyFlagset) override {}
	GLBasicSkybox(SkyCubeNode* node);
	~GLBasicSkybox() override;
};
} // namespace OpenGL
