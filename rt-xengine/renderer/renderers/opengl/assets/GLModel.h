#ifndef GLMODEL_H
#define GLMODEL_H

#include "renderer/renderers/opengl/GLAsset.h"
#include "GLMesh.h"

namespace Renderer::OpenGL
{
	struct GLRenderMesh
	{
		GLuint vao;
		std::shared_ptr<GLMesh> mesh;
	};

	// can contain multiple meshes (typically tho one but depends on the XModel)
	class GLModel : public GLAsset
	{
	protected:
		uint32 m_usage;
		std::vector<GLRenderMesh> m_renderMeshes;

	public:
		GLModel(GLRendererBase* renderer);
		virtual ~GLModel();

		bool Load(Assets::XModel* data);

		std::vector<GLRenderMesh>& GetRenderMeshes() { return m_renderMeshes; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLModel, name: " << m_associatedDescription; }
	};

}

#endif // GLMODEL_H
