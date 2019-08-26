#include "pch.h"

#include "renderer/renderers/opengl/assets/GLMesh.h"

namespace Renderer::OpenGL
{
	GLMesh::GLMesh(GLRendererBase* renderer, const std::string& name)
		: GLAsset(renderer, name), \
	      m_vbo(0),
	      m_ebo(0)
	{
	}

	GLMesh::~GLMesh()
	{
		glDeleteBuffers(1, &m_vbo);
		glDeleteBuffers(1, &m_ebo);
	}

	bool GLMesh::Load(Assets::Mesh* data, GLenum usage)
	{
		glGenBuffers(1, &m_vbo);
		glGenBuffers(1, &m_ebo);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		//glBufferData(GL_ARRAY_BUFFER, data->GetVertices().size() * sizeof(Core::Vertex),
		//	&data->GetVertices().data()[0], usage);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, data->GetVertexIndices().size() * sizeof(uint32) * 3,
		//	&data->GetVertexIndices().data()[0], usage);

		//auto materialOffsets = data->GetMaterialOffsets();
		//auto materialsInOffsetOrder = data->GetMaterialsInOffsetOrder();

		//RT_XENGINE_ASSERT(materialOffsets.size() == materialsInOffsetOrder.size());

		//auto matOffset = materialOffsets.begin();

		//auto material = materialsInOffsetOrder.begin();
		// get materials in offset order (parallel to each corresponding group)
		//for (; matOffset != materialOffsets.end() && material != materialsInOffsetOrder.end(); ++matOffset, ++material)
		{
		//	const auto begin = matOffset->first;
		//	const auto end = matOffset->second;
		//	const auto size = end - begin + 1;

		//	GLMeshGeometryGroup ggg;
		//	ggg.indicesCount = size;
		//	ggg.indicesOffset = begin;

		//	ggg.material = GetGLRenderer()->RequestGLMaterial(*material);

		//	m_groups.push_back(ggg);

		}

		return true;
	}
}
