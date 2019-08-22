#ifndef GLTESTGEOMETRY_H
#define GLTESTGEOMETRY_H

#include "system/shared/Shared.h"

#include "GLTestRenderer.h"
#include "renderer/NodeObserver.h"
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

#endif // GLTESTGEOMETRY_H
