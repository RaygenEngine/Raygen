#pragma once

#include "renderer/NodeObserver.h"
#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "renderer/renderers/opengl/assets/GLModel.h"

namespace OpenGL
{
	struct GLTestGeometry : public NodeObserver<GLTestRenderer, GeometryNode>
	{
		GLModel* glModel;

		GLTestGeometry(GLTestRenderer* renderer, GeometryNode* node);
		virtual ~GLTestGeometry() override;
	};
}
