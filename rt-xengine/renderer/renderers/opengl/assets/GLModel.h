#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLMesh.h"
#include "assets/model/Model.h"

#include "GLAD/glad.h"

namespace Renderer::OpenGL
{
	struct GLRenderMesh
	{
		GLuint vao;
		std::shared_ptr<GLMesh> mesh;
	};

	// can contain multiple meshes (typically tho one but depends on the Model)
	class GLModel : public GLAsset
	{
	protected:
		uint32 m_usage;
		std::vector<GLRenderMesh> m_renderMeshes;

	public:
		GLModel(GLRendererBase* renderer, const std::string& name);
		virtual ~GLModel();

		bool Load(Assets::Model* data);

		std::vector<GLRenderMesh>& GetRenderMeshes() { return m_renderMeshes; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLModel, name: " << m_name; }
	};

}
