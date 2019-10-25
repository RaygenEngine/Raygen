#include "pch/pch.h"

#include "renderer/renderers/opengl/basic/GLBasicAmbient.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "asset/AssetManager.h"

namespace ogl {
GLBasicAmbient::GLBasicAmbient(AmbientNode* node)
	: NodeObserver<AmbientNode, GLRendererBase>(node)
{
	ReloadSkybox();
}

void GLBasicAmbient::DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset)
{
	using DF = AmbientNode::DF;

	if (nodeDirtyFlagset[DF::SkyTexture]) {
		ReloadSkybox();
	}
}
void GLBasicAmbient::ReloadSkybox()
{
	if (node && node->GetSkybox().HasBeenAssigned()) {
		texture = GetGLAssetManager(this)->GpuGetOrCreate<GLTexture>(node->GetSkybox());
	}
}
} // namespace ogl
