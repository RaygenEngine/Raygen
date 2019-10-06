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

		virtual void DirtyNodeUpdate(std::bitset<64> nodeDirtyFlagset) override 
		{
			if (nodeDirtyFlagset[Node::DF::Properties])
			{
				// WIP:
				ReloadModel();
			}
		}

		virtual ~GLBasicGeometry() = default;
	};
}
