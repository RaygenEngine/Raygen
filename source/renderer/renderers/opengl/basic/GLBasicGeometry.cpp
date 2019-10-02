#include "pch.h"

#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "asset/AssetManager.h"

namespace OpenGL
{
	GLBasicGeometry::GLBasicGeometry(GeometryNode* node)
		: NodeObserver<GeometryNode, GLRendererBase>(node)
	{
		ReloadModel();
	}

	void GLBasicGeometry::ReloadModel()
	{
		glModel = GetGLAssetManager(this)->GetOrMakeFromUri<GLModel>(Engine::GetAssetManager()->GetPodPath(GetNode()->GetModel()));
	}
}
