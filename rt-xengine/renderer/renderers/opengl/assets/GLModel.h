#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLMesh.h"
#include "assets/model/Model.h"

#include "GLAD/glad.h"

namespace Renderer::OpenGL
{
	// can contain multiple meshes (typically tho one but depends on the Model)
	class GLModel : public GLAsset
	{
	protected:
		uint32 m_usage;

		std::vector<std::unique_ptr<GLMesh>> m_meshes;

	public:
		GLModel(GLRendererBase* renderer, const std::string& name);
		virtual ~GLModel() = default;

		bool Load(Assets::Model* data);

		const std::vector<std::unique_ptr<GLMesh>>& GetGLMeshes() const { return m_meshes; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLModel, name: " << m_name; }
	};

}
