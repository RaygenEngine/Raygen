#include "pch/pch.h"

#include "renderer/renderers/opengl/assets/GLShader.h"
#include "asset/AssetManager.h"
#include "asset/pods/StringPod.h"
#include "asset/AssetPod.h"

#include <glm/gtc/type_ptr.hpp>

namespace ogl {
GLShader::~GLShader()
{
	glDeleteProgram(programId);
}

void GLShader::Load()
{

	if (!firstLoad) {
		AssetManager::Reload(podHandle);
		AssetManager::Reload(podHandle->fragment);
		// WIP:
		if (podHandle.Lock()->geometry.podId) {
			AssetManager::Reload(podHandle->geometry);
		}

		AssetManager::Reload(podHandle->vertex);
	}
	firstLoad = false;

	auto CreateShader = [](GLenum type, PodHandle<StringPod> pod) -> GLuint {
		GLint result = GL_FALSE;
		int32 infoLogLength;

		const GLuint shaderId = glCreateShader(type);

		char const* sourcePointer = pod.Lock()->data.c_str();
		// Compile Shader
		glShaderSource(shaderId, 1, &sourcePointer, NULL);
		glCompileShader(shaderId);
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0) {
			std::vector<char> shaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(shaderId, infoLogLength, NULL, &shaderErrorMessage[0]);
			LOG_WARN("Error in {}:\n{}", AssetManager::GetEntry(pod)->name, &shaderErrorMessage[0]);
			return 0u;
		}

		return shaderId;
	};

	auto shaderPod = podHandle.Lock();

	auto vertexShaderId = CreateShader(GL_VERTEX_SHADER, shaderPod->vertex);

	// WIP:
	auto geometryShaderId = 0u;
	if (shaderPod->geometry.podId) {
		geometryShaderId = CreateShader(GL_GEOMETRY_SHADER, shaderPod->geometry);
	}

	auto fragmentShaderId = CreateShader(GL_FRAGMENT_SHADER, shaderPod->fragment);

	programId = glCreateProgram();
	glAttachShader(programId, vertexShaderId);
	if (geometryShaderId)
		glAttachShader(programId, geometryShaderId);
	glAttachShader(programId, fragmentShaderId);
	glLinkProgram(programId);

	GLint result = GL_FALSE;
	int32 infoLogLength;

	glGetProgramiv(programId, GL_LINK_STATUS, &result);
	glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) {
		std::vector<char> programErrorMessage(infoLogLength + 1);
		glGetProgramInfoLog(programId, infoLogLength, NULL, &programErrorMessage[0]);
		LOG_WARN("Error in {}:\n{}", AssetManager::GetEntry(podHandle)->name, &programErrorMessage[0]);
		return;
	}

	glDetachShader(programId, vertexShaderId);
	if (geometryShaderId)
		glDetachShader(programId, geometryShaderId);
	glDetachShader(programId, fragmentShaderId);

	glDeleteShader(vertexShaderId);
	if (geometryShaderId)
		glDeleteShader(geometryShaderId);
	glDeleteShader(fragmentShaderId);
}


void GLShader::UploadInt(const std::string& uniformName, int i)
{
	glUniform1i(uniformLocations.at(uniformName), i);
}

void GLShader::UploadFloat(const std::string& uniformName, float f)
{
	glUniform1f(uniformLocations.at(uniformName), f);
}

void GLShader::UploadVec2(const std::string& uniformName, glm::vec2 v)
{
	glUniform2fv(uniformLocations.at(uniformName), 1, glm::value_ptr(v));
}

void GLShader::UploadVec3(const std::string& uniformName, glm::vec3 v)
{
	glUniform3fv(uniformLocations.at(uniformName), 1, glm::value_ptr(v));
}

void GLShader::UploadVec4(const std::string& uniformName, glm::vec4 v)
{
	glUniform4fv(uniformLocations.at(uniformName), 1, glm::value_ptr(v));
}

void GLShader::UploadMat3(const std::string& uniformName, const glm::mat3& m, GLboolean transpose)
{
	glUniformMatrix3fv(uniformLocations.at(uniformName), 1, transpose, glm::value_ptr(m));
}

void GLShader::UploadMat4(const std::string& uniformName, const glm::mat4& m, GLboolean transpose)
{
	glUniformMatrix4fv(uniformLocations.at(uniformName), 1, transpose, glm::value_ptr(m));
}

void GLShader::AddUniform(const std::string& uniformName)
{
	uniformLocations.insert({ uniformName, glGetUniformLocation(programId, uniformName.c_str()) });
}
} // namespace ogl
