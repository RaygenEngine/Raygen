#include "pch/pch.h"

#include "renderer/renderers/opengl/basic/GLBasicSkybox.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "asset/AssetManager.h"

namespace ogl {
GLBasicSkybox::GLBasicSkybox(SkyboxNode* node)
	: NodeObserver<SkyboxNode, GLRendererBase>(node)
{
	ReloadSkybox();
}

void GLBasicSkybox::DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset)
{
	using DF = SkyboxNode::DF;

	if (nodeDirtyFlagset[DF::SkyTexture]) {
		ReloadSkybox();
	}
}
void GLBasicSkybox::ReloadSkybox()
{
	if (node && node->GetSkyMap().HasBeenAssigned()) {
		texture = GetGLAssetManager(this)->GpuGetOrCreate<GLTexture>(node->GetSkyMap());
	}
}
} // namespace ogl
