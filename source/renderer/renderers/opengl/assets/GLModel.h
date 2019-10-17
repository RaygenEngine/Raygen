#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "asset/pods/ModelPod.h"

#include <glad/glad.h>

namespace ogl {
struct GLModel : GLAsset<ModelPod> {
	struct GLMesh {
		std::string name;

		GLuint vao{ 0u };
		GLuint ebo{ 0u };

		GLuint verticesVBO{ 0u };

		GLMaterial* material{ nullptr };

		GLsizei indicesCount{ 0u };

		GLint geometryMode{ 0u };
	};
	std::vector<GLMesh> meshes;

	GLModel(PodHandle<ModelPod> handle)
		: GLAsset(handle)
	{
	}

	~GLModel() override;

protected:
	void LoadGLMesh(const ModelPod& model, GLMesh& mesh, const GeometryGroup& data);
	void Load() override;
};

} // namespace ogl
