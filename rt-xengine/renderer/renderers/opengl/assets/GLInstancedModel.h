#ifndef GLINSTANCEDMODEL_H
#define GLINSTANCEDMODEL_H

#include "GLModel.h"

namespace Renderer::OpenGL
{

	class GLInstancedModel : public GLModel
	{
		uint32 m_instanceCount;
		GLuint m_instanceMatricesVbo;

	public:
		GLInstancedModel(GLRendererBase* renderer);
		~GLInstancedModel();

		bool Load(Assets::XModel* data, World::TriangleModelInstancedGeometryNode* nodeInstancer);

		uint32 GetInstancesCount() const { return m_instanceCount; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLModel, name: " << m_associatedDescription; }
	};

}

#endif // GLINSTANCEDMODEL_H
