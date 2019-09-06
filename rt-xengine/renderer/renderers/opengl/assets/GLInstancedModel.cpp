#include "pch.h"

#include "renderer/renderers/opengl/assets/GLInstancedModel.h"

namespace OpenGL
{
	GLInstancedModel::GLInstancedModel(GLAssetManager* glAssetManager, const std::string& name)
		: GLModel(glAssetManager, name),
		  m_instanceCount(0),
		  m_instanceMatricesVbo(0)
	{
	}

	GLInstancedModel::~GLInstancedModel()
	{
		glDeleteBuffers(1, &m_instanceMatricesVbo);
	}

	bool GLInstancedModel::Load(TriangleModelInstancedGeometryNode* nodeInstancer)
	{
		// if base model is not loaded
		//if (!GLModel::Load(nodeInstancer->GetModel()))
		//	return false;

		m_instanceCount = nodeInstancer->GetInstanceGroup().GetCount();

		// bake to each mesh vao the instancing data 
		//for (auto& mesh : m_renderMeshes)
		//{
		//	glBindVertexArray(mesh.vao);

		//	// vertex Buffer Object
		//	glGenBuffers(1, &m_instanceMatricesVbo);
		//	glBindBuffer(GL_ARRAY_BUFFER, m_instanceMatricesVbo);
		//	glBufferData(GL_ARRAY_BUFFER, m_instanceCount * sizeof(glm::mat4),
		//		&nodeInstancer->GetInstanceGroup().GetMatricesBlock().data()[0], m_usage);

		//	glEnableVertexAttribArray(3);
		//	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)0);
		//	glEnableVertexAttribArray(4);
		//	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeof(glm::vec4)));
		//	glEnableVertexAttribArray(5);
		//	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(2 * sizeof(glm::vec4)));
		//	glEnableVertexAttribArray(6);
		//	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(3 * sizeof(glm::vec4)));

		//	// one instancedMatrix per mesh
		//	glVertexAttribDivisor(3, 1);
		//	glVertexAttribDivisor(4, 1);
		//	glVertexAttribDivisor(5, 1);
		//	glVertexAttribDivisor(6, 1);

		//	glBindVertexArray(0);
		//}

		return true;
	}
}
