#include "pch.h"

#include "renderer/renderers/opengl/assets/GLShader.h"
#include "asset/AssetManager.h"
#include "asset/pods/StringPod.h"
#include "asset/AssetPod.h"

namespace OpenGL
{
	GLShader::~GLShader()
	{
		glDeleteProgram(id);
	}
	
	bool GLShader::Load()
	{
		
		const auto sources = AssetManager::GetOrCreate<ShaderPod>(m_assetManagerPodPath);
		Engine::GetAssetManager()->Reload(sources);
		Engine::GetAssetManager()->Reload(sources->fragment);
		Engine::GetAssetManager()->Reload(sources->vertex);

		

		GLint result = GL_FALSE;
		int32 infoLogLength;

		const GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);

		// Compile Vertex Shader
		char const* vertexSourcePointer = sources->vertex->data.c_str();
		glShaderSource(vertexShaderID, 1, &vertexSourcePointer, NULL);
		glCompileShader(vertexShaderID); // Check Vertex Shader
		glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
		glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &vertexShaderErrorMessage[0]);
			LOG_WARN("Vertex shader error in {}:\n{}", m_assetManagerPodPath, &vertexShaderErrorMessage[0]);
			return false;
		}
		const GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		// Compile Fragment Shader
		//LOG_TRACE("Compiling shader : {}", fragmentSource->GetFileName());
		char const* fragmentSourcePointer = sources->fragment->data.c_str();
		glShaderSource(fragmentShaderID, 1, &fragmentSourcePointer, NULL);
		glCompileShader(fragmentShaderID); // Check Fragment Shader
		glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
		glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			std::vector<char> fragmentShaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);
			LOG_WARN("Fragment shader error in {}:\n{}", m_assetManagerPodPath, &fragmentShaderErrorMessage[0]);
			return false;
		}

		id = glCreateProgram();
		glAttachShader(id, vertexShaderID);
		glAttachShader(id, fragmentShaderID);
		glLinkProgram(id);

		glGetProgramiv(id, GL_LINK_STATUS, &result);
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			std::vector<char> programErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(id, infoLogLength, NULL, &programErrorMessage[0]);
			LOG_WARN("Program shader error in {}:\n{}", m_assetManagerPodPath, &programErrorMessage[0]);
			return false;
		}

		glDetachShader(id, vertexShaderID);
		glDetachShader(id, fragmentShaderID);
		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);

		return true;
	}

	void GLShader::operator+=(const std::string& uniformName)
	{
		uniformLocations.insert({ uniformName, glGetUniformLocation(id, uniformName.c_str()) });
	}
	
	GLint GLShader::operator[](const std::string& uniformName) const
	{
		return uniformLocations.at(uniformName);
	}
}
