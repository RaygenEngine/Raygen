#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestGeometry.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

namespace OpenGL
{
	GLTestGeometry::GLTestGeometry(GLTestRenderer* renderer, TriangleModelGeometryNode* node)
		: NodeObserver<GLTestRenderer, TriangleModelGeometryNode>(node)
	{
		glModel = GetGLAssetManager(this)->RequestLoadAsset<GLModel>(node->GetModel()->GetUri());
	}
}
