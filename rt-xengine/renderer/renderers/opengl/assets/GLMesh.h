#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "assets/model/Mesh.h"

#include "GLAD/glad.h"

namespace Renderer::OpenGL
{
	class GLMesh : public GLAsset
	{
		GLuint m_vao;
		GLuint m_ebo;

		// TODO: may use single in the future
		GLuint m_positionsVBO;
		GLuint m_normalsVBO;
		GLuint m_tangentsVBO;
		GLuint m_bitangentsVBO;
		GLuint m_textCoords0VBO;
		GLuint m_textCoords1VBO;
		
		GLMaterial m_material;

		GLint m_geometryMode;

		uint32 m_count;
		
	public:
		GLMesh(GLRendererBase* renderer, const std::string& name);
		~GLMesh();
		GLMesh(GLMesh&&) = default;
		GLMesh(const GLMesh&) = default;
		GLMesh& operator=(GLMesh&&) = default;
		GLMesh& operator=(const GLMesh&) = default;
		
		bool Load(Assets::GeometryGroup* data, GLenum usage);

		const GLMaterial& GetMaterial() const { return m_material; }

		GLuint GetVAO() const { return m_vao; }

		uint32 GetCount() const { return m_count; }
		 
		void ToString(std::ostream& os) const override { os << "asset-type: GLMesh, name: " << m_name; }
	};

}
