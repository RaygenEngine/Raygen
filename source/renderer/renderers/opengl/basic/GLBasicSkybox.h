#pragma once

#include "world/nodes/sky/SkyboxNode.h"
#include "renderer/renderers/opengl/GLRendererBase.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"

#include <glad/glad.h>


namespace ogl {
struct GLBasicSkybox : NodeObserver<SkyboxNode, GLRendererBase> {

	GLTexture* texture;

	GLBasicSkybox(SkyboxNode* node);


	void DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset) override;

	void ReloadSkybox();
};
} // namespace ogl
