#include "pch/pch.h"

#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/GLUtil.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

namespace OpenGL {
GLModel::~GLModel()
{
	for (auto& mesh : meshes) {
		glDeleteBuffers(1, &mesh.verticesVBO);
		glDeleteBuffers(1, &mesh.ebo);

		glDeleteVertexArrays(1, &mesh.vao);
	}
}

bool GLModel::LoadGLMesh(const ModelPod& model, GLMesh& glMesh, const GeometryGroup& data, GLenum usage)
{
	glMesh.geometryMode = GetGLGeometryMode(data.mode);

	glGenVertexArrays(1, &glMesh.vao);
	glBindVertexArray(glMesh.vao);


	glGenBuffers(1, &glMesh.verticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, glMesh.verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(VertexData), &data.vertices[0], usage);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, tangent));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, bitangent));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, textCoord0));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, textCoord1));


	glGenBuffers(1, &glMesh.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(data.indices[0]), &data.indices[0], usage);

	glMesh.count = static_cast<GLsizei>(data.indices.size());

	auto materialHandle = model.materials[data.materialIndex];
	glMesh.material = GetGLAssetManager(this)->GpuGetOrCreate<GLMaterial>(materialHandle);

	DebugBoundVAO("name");

	glBindVertexArray(0);

	return true;
}

bool GLModel::Load()
{
	auto modelData = podHandle.Lock();
	TIMER_STATIC_SCOPE("uploading model time");

	// TODO
	// m_usage = GetGLUsage(data->GetUsage());
	usage = GL_STATIC_DRAW;

	for (auto& mesh : modelData->meshes) {
		for (auto& geometryGroup : mesh.geometryGroups) {
			GLMesh& glmesh = meshes.emplace_back(GLMesh());
			if (!LoadGLMesh(*modelData, glmesh, geometryGroup, usage)) {
				return false;
			}
		}
	}

	return true;
}
} // namespace OpenGL
