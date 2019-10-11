#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "asset/pods/ModelPod.h"

#include <glad/glad.h>

namespace OpenGL {
struct GLModel : GLAsset<ModelPod> {
	struct GLMesh {
		std::string name;

		GLuint vao{ 0u };
		GLuint ebo{ 0u };

		GLuint verticesVBO{ 0u };

		GLMaterial* material{ nullptr };
		GLint geometryMode{ 0u };

		GLsizei count{ 0u };
	};

	GLenum usage{ GL_STATIC_DRAW };
	std::vector<GLMesh> meshes;

	GLModel(PodHandle<ModelPod> handle)
		: GLAsset(handle)
	{
	}

	~GLModel() override;

protected:
	bool LoadGLMesh(const ModelPod& model, GLMesh& mesh, const GeometryGroup& data, GLenum usage);
	bool Load() override;
};

} // namespace OpenGL
