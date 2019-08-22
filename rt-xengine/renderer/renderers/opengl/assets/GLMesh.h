#ifndef GLMESH_H
#define GLMESH_H

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLMaterial.h"

namespace Renderer::OpenGL
{

	struct GLMeshGeometryGroup
	{
		uint32 indicesOffset;
		uint32 indicesCount;
		std::shared_ptr<GLMaterial> material;
	};

	class GLMesh : public GLAsset
	{
		GLuint m_vbo;
		GLuint m_ebo;

		std::vector<GLMeshGeometryGroup> m_groups;

	public:
		GLMesh(GLRendererBase* renderer);
		~GLMesh();

		bool Load(Assets::XMesh* data, GLenum usage);

		const std::vector<GLMeshGeometryGroup>& GetGeometryGroups() const { return m_groups; }

		GLuint GetVBO() const { return m_vbo; };
		GLuint GetEBO() const { return m_ebo; };

		void ToString(std::ostream& os) const override { os << "asset-type: GLMesh, name: " << m_associatedDescription; }
	};

}

#endif // GLMESH_H
