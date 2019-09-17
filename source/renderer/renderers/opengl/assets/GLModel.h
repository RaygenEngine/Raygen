#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLMaterial.h"

#include "GLAD/glad.h"
#include <optional>
#include "asset/pods/ModelPod.h"

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

			GLsizei count{ 0u };
		};

	private:

		bool LoadGLMesh(GLMesh& mesh, GeometryGroup& data, GLenum usage);
		

		GLenum m_usage;

		std::vector<GLMesh> m_meshes;

	public:
		GLModel(const fs::path& assocPath)
			: GLAsset(assocPath),
			  m_usage(GL_STATIC_DRAW) {}
		bool Load() override;

		friend class GLAssetManager;
	public:
		virtual ~GLModel();

		std::vector<GLMesh>& GetGLMeshes() { return m_meshes; }

		
	};

}
