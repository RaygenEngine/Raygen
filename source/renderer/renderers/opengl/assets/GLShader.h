#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "asset/pods/ShaderPod.h"

#include <glad/glad.h>

namespace ogl {
struct GLShader : GLAsset<ShaderPod> {
	GLuint programId{ 0 };
	bool firstLoad{ true };

	GLShader(PodHandle<ShaderPod> handle)
		: GLAsset(handle)
	{
	}

	~GLShader() override;


	void Load() override;


	void AddUniform(const std::string& uniformName);
	void UploadInt(const std::string& uniformName, int i);
	void UploadFloat(const std::string& uniformName, float f);
	void UploadVec2(const std::string& uniformName, glm::vec2 v);
	void UploadVec3(const std::string& uniformName, glm::vec3 v);
	void UploadVec4(const std::string& uniformName, glm::vec4 v);

	void UploadMat4(const std::string& uniformName, const glm::mat4& m, GLboolean transpose = GL_FALSE);
	void UploadMat3(const std::string& uniformName, const glm::mat3& m, GLboolean transpose = GL_FALSE);

protected:
	// temporary
	std::unordered_map<std::string, GLint> uniformLocations;
};

} // namespace ogl
