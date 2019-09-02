#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestGeometry.h"

namespace Renderer::OpenGL
{
	GLTestGeometry::GLTestGeometry(GLTestRenderer* renderer, World::TriangleModelGeometryNode* node)
		: TypedNodeObserver<GLTestRenderer, World::TriangleModelGeometryNode>(renderer, node)
	{
		glModel = GetRenderer()->GetGLAssetManager()->RequestGLModel(node->GetModel());
	}
}
