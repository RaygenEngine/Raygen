#pragma once

#include "renderer/renderers/opengl/assets/GLModel.h"
#include "world/nodes/geometry/TriangleModelInstancedGeometryNode.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	//class GLInstancedModel : public GLModel
	//{
	//	uint32 m_instanceCount;
	//	GLuint m_instanceMatricesVbo;

	//public:
	//	GLInstancedModel(const std::string& name)
	//		: GLModel(name),
	//		  m_instanceCount(0),
	//		  m_instanceMatricesVbo(0) {}
	//	~GLInstancedModel();

	//	bool Load(TriangleModelInstancedGeometryNode* nodeInstancer);

	//	uint32 GetInstancesCount() const { return m_instanceCount; }

	//	void ToString(std::ostream& os) const override { os << "asset-type: GLModel, name: " << m_name; }
	//};

}
