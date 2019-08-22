#include "GLTestGeometry.h"

namespace Renderer::OpenGL
{
	GLTestGeometry::GLTestGeometry(GLTestRenderer* renderer, World::TriangleModelGeometryNode* node)
		: TypedNodeObserver<GLTestRenderer, World::TriangleModelGeometryNode>(renderer, node)
	{
		glModel = GetRenderer()->RequestGLModel(node->GetModel());
	}
}
