#include "pch.h"

#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/GLUtil.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

namespace OpenGL
{

	std::optional<GLModel::GLMesh> GLModel::LoadGLMesh(GeometryGroup& data, GLenum usage)
	{
		GLMesh glMesh{};

		glMesh.geometryMode = GetGLGeometryMode(data.mode);

		glGenVertexArrays(1, &glMesh.vao);
		glBindVertexArray(glMesh.vao);

		const auto CreateUploadBuffer = [](GLuint& bufferId, auto& data, GLenum usage)
		{
			glGenBuffers(1, &bufferId);
			glBindBuffer(GL_ARRAY_BUFFER, bufferId);
			glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(data[0]),
				&data[0], usage);
		};

		CreateUploadBuffer(glMesh.positionsVBO, data.positions, usage);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		CreateUploadBuffer(glMesh.normalsVBO, data.normals, usage);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		CreateUploadBuffer(glMesh.tangentsVBO, data.tangents, usage);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

		CreateUploadBuffer(glMesh.bitangentsVBO, data.bitangents, usage);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		CreateUploadBuffer(glMesh.textCoords0VBO, data.textCoords0, usage);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		CreateUploadBuffer(glMesh.textCoords1VBO, data.textCoords1, usage);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &glMesh.ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(data.indices[0]),
			&data.indices[0], usage);

		glMesh.count = data.indices.size();

		Engine::GetAssetManager()->RefreshPod(data.material);
		
		glMesh.material = GetGLAssetManager(this)->RequestLoadAsset<GLMaterial>(Engine::GetAssetManager()->GetPodPath(data.material));

		DebugBoundVAO("name");

		glBindVertexArray(0);

		return glMesh;
	}

	GLModel::~GLModel()
	{
		for (auto& mesh : m_meshes)
		{
			glDeleteBuffers(1, &mesh.positionsVBO);
			glDeleteBuffers(1, &mesh.normalsVBO);
			glDeleteBuffers(1, &mesh.tangentsVBO);
			glDeleteBuffers(1, &mesh.bitangentsVBO);
			glDeleteBuffers(1, &mesh.textCoords0VBO);
			glDeleteBuffers(1, &mesh.textCoords1VBO);
			glDeleteBuffers(1, &mesh.ebo);

			glDeleteVertexArrays(1, &mesh.vao);
		}
	}

	bool GLModel::Load()
	{
		auto modelData = Engine::GetAssetManager()->RequestFreshPod<ModelPod>(m_assetManagerPodPath);
		
		TIMER_STATIC_SCOPE("uploading model time");

		//m_usage = GetGLUsage(data->GetUsage());
		m_usage = GL_STATIC_DRAW;

		for (auto& mesh : modelData->meshes)
		{
			for (auto& geometryGroup : mesh.geometryGroups)
			{
				auto glMesh = LoadGLMesh(geometryGroup, m_usage);
				if (!glMesh)
					return false;
				
				m_meshes.emplace_back(glMesh.value());
			}
		}

		return true;
	}

	void GLModel::Unload()
	{
		for (auto& mesh : m_meshes)
		{
			glDeleteBuffers(1, &mesh.positionsVBO);
			glDeleteBuffers(1, &mesh.normalsVBO);
			glDeleteBuffers(1, &mesh.tangentsVBO);
			glDeleteBuffers(1, &mesh.bitangentsVBO);
			glDeleteBuffers(1, &mesh.textCoords0VBO);
			glDeleteBuffers(1, &mesh.textCoords1VBO);
			glDeleteBuffers(1, &mesh.ebo);

			glDeleteVertexArrays(1, &mesh.vao);
		}
	}
}
