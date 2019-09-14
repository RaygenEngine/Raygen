#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "asset/assets/ModelAsset.h"

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

		bool LoadGLMesh(GLMesh& mesh, GeometryGroupPod& data, GLenum usage);
		
		ModelAsset* m_model;
		GLenum m_usage;

		std::vector<GLMesh> m_meshes;

		GLModel(ModelAsset* model)
			: m_model(model),
			  m_usage(GL_STATIC_DRAW) {}
		bool Load() override;

		friend class GLAssetManager;
	public:
		virtual ~GLModel();

		std::vector<GLMesh>& GetGLMeshes() { return m_meshes; }

		
	};

}
