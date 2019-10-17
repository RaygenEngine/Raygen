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


	void StoreUniformLoc(const std::string& uniformName);

	void SendTexture(const std::string& uniformName, GLuint textureId, int32 textureUnitOffset);
	void SendTexture(GLuint textureId, int32 textureUnitOffset);
	void SendCubeTexture(const std::string& uniformName, GLuint textureId, int32 textureUnitOffset);
	void SendCubeTexture(GLuint textureId, int32 textureUnitOffset);
	void SendInt(const std::string& uniformName, int32 i);
	void SendFloat(const std::string& uniformName, float f);
	void SendVec2(const std::string& uniformName, glm::vec2 v);
	void SendVec3(const std::string& uniformName, glm::vec3 v);
	void SendVec4(const std::string& uniformName, glm::vec4 v);

	void SendMat4(const std::string& uniformName, const glm::mat4& m, GLboolean transpose = GL_FALSE);
	void SendMat3(const std::string& uniformName, const glm::mat3& m, GLboolean transpose = GL_FALSE);

protected:
	// temporary
	std::unordered_map<std::string, GLint> uniformLocations;
};

} // namespace ogl
