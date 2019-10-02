#pragma once

#include "world/nodes/geometry/GeometryNode.h"
#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/GLRendererBase.h"

namespace OpenGL
{
	struct GLBasicGeometry : NodeObserver<GeometryNode, GLRendererBase>
	{
		GLModel* glModel;
		
		GLBasicGeometry(GeometryNode* node);

		void ReloadModel();
	};
}
