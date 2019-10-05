#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "asset/pods/ModelPod.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	struct GLModel : GLAsset
	{		
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

		GLenum usage{ GL_STATIC_DRAW };
		std::vector<GLMesh> meshes;

		GLModel(const uri::Uri& assocPath)
			: GLAsset(assocPath) {}
		virtual ~GLModel();

	protected:
		bool LoadGLMesh(PodHandle<ModelPod> model, GLMesh& mesh, GeometryGroup& data, GLenum usage);
		bool Load() override;
	};

}
