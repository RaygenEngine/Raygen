#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestGeometry.h"

namespace OpenGL
{
	GLTestGeometry::GLTestGeometry(GLTestRenderer* renderer, TriangleModelGeometryNode* node)
		: TypedNodeObserver<GLTestRenderer, TriangleModelGeometryNode>(renderer, node)
	{
		glModel = GetRenderer()->GetGLAssetManager()->RequestGLModel(node->GetModel());
	}
}
