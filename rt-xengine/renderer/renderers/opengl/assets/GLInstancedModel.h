#pragma once

#include "renderer/renderers/opengl/assets/GLModel.h"
#include "world/nodes/geometry/TriangleModelInstancedGeometryNode.h"

#include "GLAD/glad.h"

namespace Renderer::OpenGL
{

	class GLInstancedModel : public GLModel
	{
		uint32 m_instanceCount;
		GLuint m_instanceMatricesVbo;

	public:
		GLInstancedModel(GLAssetManager* glAssetManager, const std::string& name);
		~GLInstancedModel();

		bool Load(World::TriangleModelInstancedGeometryNode* nodeInstancer);

		uint32 GetInstancesCount() const { return m_instanceCount; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLModel, name: " << m_name; }
	};

}
