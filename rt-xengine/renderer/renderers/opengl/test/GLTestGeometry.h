#pragma once

#include "renderer/NodeObserver.h"
#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "world/nodes/geometry/TriangleModelGeometryNode.h"

namespace OpenGL
{
	struct GLTestGeometry : TypedNodeObserver<GLTestRenderer, TriangleModelGeometryNode>
	{
		std::shared_ptr<GLModel> glModel;

		GLTestGeometry(GLTestRenderer* renderer, TriangleModelGeometryNode* node);
		~GLTestGeometry() = default;
	};
}
