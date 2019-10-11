#include "pch/pch.h"

#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "asset/AssetManager.h"

namespace ogl {
GLBasicGeometry::GLBasicGeometry(GeometryNode* node)
	: NodeObserver<GeometryNode, GLRendererBase>(node)
{
	ReloadModel();
}

void GLBasicGeometry::ReloadModel()
{
	glModel = GetGLAssetManager(this)->GpuGetOrCreate<GLModel>(node->GetModel());
}
} // namespace ogl
