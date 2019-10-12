#pragma once

#include "world/nodes/geometry/GeometryNode.h"
#include "renderer/renderers/opengl/GLRendererBase.h"
#include "renderer/renderers/opengl/assets/GLModel.h"

namespace ogl {
struct GLBasicGeometry : NodeObserver<GeometryNode, GLRendererBase> {
	GLModel* glModel;

	GLBasicGeometry(GeometryNode* node);

	void ReloadModel();

	virtual void DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset) override
	{
		if (nodeDirtyFlagset[Node::DF::Properties]) {
			ReloadModel();
		}
	}
};
} // namespace ogl
