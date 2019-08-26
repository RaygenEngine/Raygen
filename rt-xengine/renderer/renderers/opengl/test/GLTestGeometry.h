#pragma once

#include "renderer/NodeObserver.h"
#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "world/nodes/geometry/TriangleModelGeometryNode.h"

namespace Renderer::OpenGL
{
	struct GLTestGeometry : TypedNodeObserver<GLTestRenderer, World::TriangleModelGeometryNode>
	{
		std::shared_ptr<GLModel> glModel;

		GLTestGeometry(GLTestRenderer* renderer, World::TriangleModelGeometryNode* node);
		~GLTestGeometry() = default;
	};
}
