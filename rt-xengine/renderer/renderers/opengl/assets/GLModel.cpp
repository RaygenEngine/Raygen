#include "pch.h"

#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/GLUtil.h"
#include "renderer/renderers/opengl/GLRendererBase.h"

namespace Renderer::OpenGL
{
	GLModel::GLModel(GLRendererBase* renderer, const std::string& name)
		: GLAsset(renderer, name),
	      m_usage(GL_STATIC_DRAW)
	{
	}

	GLModel::~GLModel()
	{
		for (auto& rm : m_renderMeshes)
			glDeleteVertexArrays(1, &rm.vao);
	}

	bool GLModel::Load(Assets::Model* data)
	{
		m_usage = GetGLUsage(data->GetUsage());

		for (auto& mesh : data->GetMeshes())
		{
			GLRenderMesh grm;
			glGenVertexArrays(1, &grm.vao);

			glBindVertexArray(grm.vao);

			//grm.mesh = GetGLRenderer()->RequestGLMesh(mesh, m_usage);

			//glBindBuffer(GL_ARRAY_BUFFER, grm.mesh->GetVBO());

			//// vertex positions
			//glEnableVertexAttribArray(0);
			//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Core::Vertex), (void*)0);

			//// vertex normals
			//glEnableVertexAttribArray(1);
			//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Core::Vertex), (void*)offsetof(Core::Vertex, normal));

			//// vertex texture coords
			//glEnableVertexAttribArray(2);
			//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Core::Vertex), (void*)offsetof(Core::Vertex, uv));

			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grm.mesh->GetEBO());

			DebugBoundVAO(m_name);

			glBindVertexArray(0);

			m_renderMeshes.push_back(grm);
		}

		return true;
	}
}
