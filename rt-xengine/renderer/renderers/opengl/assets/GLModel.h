#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "assets/model/ModelAsset.h"

#include "GLAD/glad.h"
#include <optional>

namespace OpenGL
{
	class GLModel : public GLAsset
	{
	public:
		struct GLMesh
		{
			std::string name;
			
			GLuint vao{ 0u };
			GLuint ebo{ 0u };

			// TODO: may use single in the future
			GLuint positionsVBO{ 0u };
			GLuint normalsVBO{ 0u };
			GLuint tangentsVBO{ 0u };
			GLuint bitangentsVBO{ 0u };
			GLuint textCoords0VBO{ 0u };
			GLuint textCoords1VBO{ 0u };

			GLMaterial* material{ nullptr };

			GLint geometryMode{ 0u };

			uint64 count{ 0u };
		};

	private:

		std::optional<GLMesh> LoadGLMesh(const ModelAsset::Mesh::GeometryGroup& data, GLenum usage);
		
		ModelAsset* m_model;

	protected:
		GLenum m_usage;

		std::vector<GLMesh> m_meshes;

	public:
		GLModel(ModelAsset* model)
			: GLAsset(model),
		      m_model(model),
			  m_usage(GL_STATIC_DRAW) {}

		virtual ~GLModel();

		std::vector<GLMesh>& GetGLMeshes() { return m_meshes; }

	protected:
		
		bool Load() override;
		void Unload() override;
	};

}
