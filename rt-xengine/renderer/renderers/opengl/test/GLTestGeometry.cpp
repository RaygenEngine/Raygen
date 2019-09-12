#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestGeometry.h"

namespace OpenGL
{
	GLTestGeometry::GLTestGeometry(GLTestRenderer* renderer, TriangleModelGeometryNode* node)
		: NodeObserver<GLTestRenderer, TriangleModelGeometryNode>(node)
	{
		glModel = GetGLAssetManager(this)->MaybeGenerateAsset<GLModel>(node->GetModel());
		GetGLAssetManager(this)->Load(glModel);
	}
}
