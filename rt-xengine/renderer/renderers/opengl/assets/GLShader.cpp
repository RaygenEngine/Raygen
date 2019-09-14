#include "pch.h"

#include "renderer/renderers/opengl/assets/GLShader.h"
#include "asset/AssetManager.h"

namespace OpenGL
{
	GLShader::~GLShader()
	{
		glDeleteProgram(m_glId);
	}
	
	bool GLShader::Load()
	{
		auto am = Engine::GetAssetManager();

		const auto sources = am->RequestFreshPod<ShaderPod>(m_assetManagerPodPath);
		am->RefreshPod(sources->vertex);
		am->RefreshPod(sources->fragment);

		GLint Result = GL_FALSE;
		int32 infoLogLength;

		const GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);

		// Compile Vertex Shader
		//LOG_TRACE("Compiling shader : {}", vertexSource->GetFileName());
		char const* VertexSourcePointer = sources->vertex->data.c_str();
		glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
		glCompileShader(VertexShaderID); // Check Vertex Shader
		glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			std::vector<char> VertexShaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(VertexShaderID, infoLogLength, NULL, &VertexShaderErrorMessage[0]);
			//LOG_WARN("{}:\n{}", vertexSource->GetFileName(), &VertexShaderErrorMessage[0]);

			return false;
		}
		const GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		// Compile Fragment Shader
		//LOG_TRACE("Compiling shader : {}", fragmentSource->GetFileName());
		char const* FragmentSourcePointer = sources->fragment->data.c_str();
		glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
		glCompileShader(FragmentShaderID); // Check Fragment Shader
		glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			std::vector<char> FragmentShaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(FragmentShaderID, infoLogLength, NULL, &FragmentShaderErrorMessage[0]);
			//LOG_WARN("{}:\n{}", fragmentSource->GetFileName(), &FragmentShaderErrorMessage[0]);

			return false;
		}

		// Link the program
		LOG_DEBUG("Linking program");
		m_glId = glCreateProgram();
		glAttachShader(m_glId, VertexShaderID);
		glAttachShader(m_glId, FragmentShaderID);
		glLinkProgram(m_glId);

		// Check the program
		glGetProgramiv(m_glId, GL_LINK_STATUS, &Result);
		glGetProgramiv(m_glId, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			std::vector<char> ProgramErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(m_glId, infoLogLength, NULL, &ProgramErrorMessage[0]);
			LOG_WARN("\n{}", &ProgramErrorMessage[0]);

			return false;
		}

		glDetachShader(m_glId, VertexShaderID);
		glDetachShader(m_glId, FragmentShaderID);
		glDeleteShader(VertexShaderID);
		glDeleteShader(FragmentShaderID);

		return true;
	}

	void GLShader::SetUniformLocation(const std::string& name)
	{
		m_uniformLocations[name] = glGetUniformLocation(m_glId, name.c_str());
	}

	GLint GLShader::GetUniformLocation(const std::string& name)
	{
		return m_uniformLocations[name];
	}
}
