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
		GLenum m_usage;

		std::vector<GLMesh*> m_meshes;

	public:
		GLModel(GLAssetManager* glAssetManager, const std::string& name);
		//virtual ~GLModel() = default;

		bool Load(Assets::Model* data);

		std::vector<GLMesh*>& GetGLMeshes() { return m_meshes; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLModel, name: " << m_name; }
		
		virtual ~GLModel() {
			for (auto ptr : m_meshes) {
				delete ptr;
			}
		}
	};

}
