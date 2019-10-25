#pragma once

#include "world/nodes/light/AmbientNode.h"
#include "renderer/renderers/opengl/GLRendererBase.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"

namespace ogl {
struct GLBasicAmbient : NodeObserver<AmbientNode, GLRendererBase> {

	GLTexture* texture{ nullptr };

	GLBasicAmbient(AmbientNode* node);

	void DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset) override;

	void ReloadSkybox();
};
} // namespace ogl
