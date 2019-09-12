#pragma once

#include "renderer/NodeObserver.h"
#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "world/nodes/geometry/TriangleModelGeometryNode.h"
#include "renderer/renderers/opengl/assets/GLModel.h"

namespace OpenGL
{
	struct GLTestGeometry : public NodeObserver<GLTestRenderer, TriangleModelGeometryNode>
	{
		GLModel* glModel;

		GLTestGeometry(GLTestRenderer* renderer, TriangleModelGeometryNode* node);
		~GLTestGeometry() = default;
	};
}
