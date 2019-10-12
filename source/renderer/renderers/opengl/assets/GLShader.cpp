#include "pch/pch.h"

#include "renderer/renderers/opengl/assets/GLShader.h"
#include "asset/AssetManager.h"
#include "asset/pods/StringPod.h"
#include "asset/AssetPod.h"

#include <glm/gtc/type_ptr.hpp>

namespace ogl {
GLShader::~GLShader()
{
	glDeleteProgram(id);
}

bool GLShader::Load()
{
	if (!firstLoad) {
		AssetManager::Reload(podHandle);
		AssetManager::Reload(podHandle->fragment);
		AssetManager::Reload(podHandle->vertex);
	}
	firstLoad = false;

	auto sources = podHandle.Lock();

	GLint result = GL_FALSE;
	int32 infoLogLength;

	const GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);

	// Compile Vertex Shader
	char const* vertexSourcePointer = sources->vertex->data.c_str();
	glShaderSource(vertexShaderID, 1, &vertexSourcePointer, NULL);
	glCompileShader(vertexShaderID); // Check Vertex Shader
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) {
		std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
		glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &vertexShaderErrorMessage[0]);
		LOG_WARN(
			"Vertex shader error in {}:\n{}", AssetManager::GetEntry(podHandle)->name, &vertexShaderErrorMessage[0]);
		return false;
	}
	const GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Compile Fragment Shader
	// LOG_TRACE("Compiling shader : {}", fragmentSource->GetFileName());
	char const* fragmentSourcePointer = sources->fragment->data.c_str();
	glShaderSource(fragmentShaderID, 1, &fragmentSourcePointer, NULL);
	glCompileShader(fragmentShaderID); // Check Fragment Shader
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) {
		std::vector<char> fragmentShaderErrorMessage(infoLogLength + 1);
		glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);
		LOG_WARN("Fragment shader error in {}:\n{}", AssetManager::GetEntry(podHandle)->name,
			&fragmentShaderErrorMessage[0]);
		return false;
	}

	id = glCreateProgram();
	glAttachShader(id, vertexShaderID);
	glAttachShader(id, fragmentShaderID);
	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &result);
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) {
		std::vector<char> programErrorMessage(infoLogLength + 1);
		glGetProgramInfoLog(id, infoLogLength, NULL, &programErrorMessage[0]);
		LOG_WARN("Program shader error in {}:\n{}", AssetManager::GetEntry(podHandle)->name, &programErrorMessage[0]);
		return false;
	}

	glDetachShader(id, vertexShaderID);
	glDetachShader(id, fragmentShaderID);
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return true;
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
	uniformLocations.insert({ uniformName, glGetUniformLocation(id, uniformName.c_str()) });
}
} // namespace ogl
