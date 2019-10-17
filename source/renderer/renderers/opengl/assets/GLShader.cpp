#include "pch/pch.h"

#include "renderer/renderers/opengl/assets/GLShader.h"
#include "asset/AssetManager.h"
#include "asset/pods/StringPod.h"
#include "asset/AssetPod.h"
#include "renderer/util/GLSLutil.h"

#include <sstream>
#include <glm/gtc/type_ptr.hpp>

namespace ogl {
GLShader::~GLShader()
{
	glDeleteProgram(programId);
}

// TODO: tidy
void GLShader::Load()
{
	// TODO: handle compile and link status
	if (!firstLoad) {
		AssetManager::Reload(podHandle);
		for (auto f : podHandle.Lock()->files)
			AssetManager::Reload(f);
	}
	firstLoad = false;

	auto CreateShader = [&](GLenum type, PodHandle<StringPod> pod) -> GLuint {
		GLint result = GL_FALSE;
		int32 infoLogLength;

		const GLuint shaderId = glCreateShader(type);

		// TODO:
		auto source = pod.Lock();

		auto originalPath = AssetManager::GetPodUri(pod);
		auto ppath = originalPath;

		size_t offset = 0;
		auto data = glsl::ProcessIncludeCommands(pod, offset);

		char const* sourcePointer = data.c_str();
		// Compile Shader
		glShaderSource(shaderId, 1, &sourcePointer, NULL);
		glCompileShader(shaderId);
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0) {
			std::string shaderErrorMessage;
			shaderErrorMessage.resize(infoLogLength);
			glGetShaderInfoLog(shaderId, infoLogLength, NULL, &shaderErrorMessage[0]);

			// PERF:

			std::string result;

			std::istringstream iss(shaderErrorMessage);

			for (std::string line; std::getline(iss, line);) {
				if (line.empty() || line == "" || line[0] == '\0') {
					break;
				}
				auto lineStartPos = line.find_first_of('(');
				auto lineEndPos = line.find_first_of(')');

				auto cutUntil = line.find_first_of(':');

				auto lineNumber = std::stoi(line.substr(lineStartPos + 1, lineEndPos - lineStartPos - 1));

				lineNumber = lineNumber - offset;

				if (lineNumber < 0) {
					result += ">>preprocessor/include-error:";
				}
				else {
					result += ">>" + std::to_string(lineNumber) + ":";
				}

				result += line.substr(cutUntil + 1) + "\n";
			}


			LOG_WARN("Error in {}:\n{}", AssetManager::GetEntry(pod)->name, &result[0]);
			return 0u;
		}


		return shaderId;
	};


	auto shaderPod = podHandle.Lock();

	programId = glCreateProgram();

	std::array<GLuint, 3> programParts{ 0 };

	for (auto f : shaderPod->files) {

		auto filePath = AssetManager::GetPodUri(f);

		if (uri::MatchesExtension(filePath, ".vert")) {
			programParts[0] = CreateShader(GL_VERTEX_SHADER, f);
		}
		else if (uri::MatchesExtension(filePath, ".geom")) {
			programParts[1] = CreateShader(GL_GEOMETRY_SHADER, f);
		}
		else if (uri::MatchesExtension(filePath, ".frag")) {
			programParts[2] = CreateShader(GL_FRAGMENT_SHADER, f);
		}
	}


	for (auto pp : programParts) {
		if (pp) {
			glAttachShader(programId, pp);
		}
	}

	glLinkProgram(programId);
	GLint result = GL_FALSE;
	int32 infoLogLength;

	glGetProgramiv(programId, GL_LINK_STATUS, &result);
	glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) {
		std::string programErrorMessage;
		programErrorMessage.resize(infoLogLength);
		// TODO: check if this needs injection
		glGetProgramInfoLog(programId, infoLogLength, NULL, &programErrorMessage[0]);
		LOG_WARN("Error in {}:\n{}", AssetManager::GetEntry(podHandle)->name, &programErrorMessage[0]);
		return;
	}

	for (auto pp : programParts) {
		if (pp) {
			glDetachShader(programId, pp);
			glDeleteShader(pp);
		}
	}
}


void GLShader::SendTexture(const std::string& uniformName, GLuint textureId, int32 textureUnitOffset)
{
	SendInt(uniformName, textureUnitOffset);
	glActiveTexture(GL_TEXTURE0 + textureUnitOffset);
	glBindTexture(GL_TEXTURE_2D, textureId);
}

void GLShader::SendTexture(GLuint textureId, int32 textureUnitOffset)
{
	glActiveTexture(GL_TEXTURE0 + textureUnitOffset);
	glBindTexture(GL_TEXTURE_2D, textureId);
}

void GLShader::SendCubeTexture(const std::string& uniformName, GLuint textureId, int32 textureUnitOffset)
{
	SendInt(uniformName, textureUnitOffset);
	glActiveTexture(GL_TEXTURE0 + textureUnitOffset);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
}

void GLShader::SendCubeTexture(GLuint textureId, int32 textureUnitOffset)
{
	glActiveTexture(GL_TEXTURE0 + textureUnitOffset);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
}

void GLShader::SendInt(const std::string& uniformName, int32 i)
{
	glUniform1i(uniformLocations.at(uniformName), i);
}

void GLShader::SendFloat(const std::string& uniformName, float f)
{
	glUniform1f(uniformLocations.at(uniformName), f);
}

void GLShader::SendVec2(const std::string& uniformName, glm::vec2 v)
{
	glUniform2fv(uniformLocations.at(uniformName), 1, glm::value_ptr(v));
}

void GLShader::SendVec3(const std::string& uniformName, glm::vec3 v)
{
	glUniform3fv(uniformLocations.at(uniformName), 1, glm::value_ptr(v));
}

void GLShader::SendVec4(const std::string& uniformName, glm::vec4 v)
{
	glUniform4fv(uniformLocations.at(uniformName), 1, glm::value_ptr(v));
}

void GLShader::SendMat3(const std::string& uniformName, const glm::mat3& m, GLboolean transpose)
{
	glUniformMatrix3fv(uniformLocations.at(uniformName), 1, transpose, glm::value_ptr(m));
}

void GLShader::SendMat4(const std::string& uniformName, const glm::mat4& m, GLboolean transpose)
{
	glUniformMatrix4fv(uniformLocations.at(uniformName), 1, transpose, glm::value_ptr(m));
}

void GLShader::StoreUniformLoc(const std::string& uniformName)
{
	uniformLocations.insert({ uniformName, glGetUniformLocation(programId, uniformName.c_str()) });
}
} // namespace ogl
