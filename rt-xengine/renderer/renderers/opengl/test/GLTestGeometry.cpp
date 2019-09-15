#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestGeometry.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "asset/AssetManager.h"

namespace OpenGL
{
	GLTestGeometry::GLTestGeometry(GLTestRenderer* renderer, TriangleModelGeometryNode* node)
		: NodeObserver<GLTestRenderer, TriangleModelGeometryNode>(node)
	{
		glModel = GetGLAssetManager(this)->GetOrMakeFromUri<GLModel>(Engine::GetAssetManager()->GetPodPath(node->GetModel()));
	}
}
