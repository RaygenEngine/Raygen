#include "pch.h"

#include "renderer/renderers/opengl/assets/GLMesh.h"
#include "renderer/renderers/opengl/GLUtil.h"

namespace Renderer::OpenGL
{
	GLMesh::GLMesh(GLAssetManager* glAssetManager, const std::string& name)
		: GLAsset(glAssetManager, name),
		  m_vao(0),
		  m_ebo(0),
		  m_positionsVBO(0),
		  m_normalsVBO(0),
		  m_tangentsVBO(0),
		  m_bitangentsVBO(0),
		  m_textCoords0VBO(0),
		  m_textCoords1VBO(0),
		  m_material(glAssetManager, name),
	      m_geometryMode(0),
	      m_count(0)
	{
	}

	GLMesh::~GLMesh()
	{
		glDeleteBuffers(1, &m_positionsVBO);
		glDeleteBuffers(1, &m_normalsVBO);
		glDeleteBuffers(1, &m_tangentsVBO);
		glDeleteBuffers(1, &m_bitangentsVBO);
		glDeleteBuffers(1, &m_textCoords0VBO);
		glDeleteBuffers(1, &m_textCoords1VBO);
		glDeleteBuffers(1, &m_ebo);

		glDeleteVertexArrays(1, &m_vao);
	}

	bool GLMesh::Load(Assets::GeometryGroup* data, GLenum usage)
	{
		m_geometryMode = GetGLGeometryMode(data->GetGeometryMode());
		
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		auto CreateUploadBuffer = [](GLuint& bufferId, auto& data, GLenum usage)
		{
			glGenBuffers(1, &bufferId);
			glBindBuffer(GL_ARRAY_BUFFER, bufferId);
			glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(data[0]),
				&data[0], usage);
		};

		CreateUploadBuffer(m_positionsVBO, data->GetPositions(), usage);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		CreateUploadBuffer(m_normalsVBO, data->GetNormals(), usage);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		CreateUploadBuffer(m_tangentsVBO, data->GetTangents(), usage);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

		CreateUploadBuffer(m_bitangentsVBO, data->GetBitangents(), usage);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		CreateUploadBuffer(m_textCoords0VBO, data->GetTextCoords0(), usage);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		CreateUploadBuffer(m_textCoords1VBO, data->GetTextCoords1(), usage);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &m_ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, data->GetIndices().size() * sizeof(data->GetIndices()[0]),
			&data->GetIndices()[0], usage);

		m_count = data->GetIndices().size();

		m_material.Load(data->GetMaterial());

		DebugBoundVAO(m_name);
		
		glBindVertexArray(0);

		return true;
	}
}
