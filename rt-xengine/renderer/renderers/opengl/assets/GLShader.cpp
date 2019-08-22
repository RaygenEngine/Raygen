#include "pch.h"
#include "GLShader.h"


namespace Renderer::OpenGL
{
	GLShader::GLShader(GLRendererBase* renderer)
		: GLAsset(renderer), m_programId(0)
	{
	}

	GLShader::~GLShader()
	{
		glDeleteProgram(m_programId);
	}

	bool GLShader::Load(Assets::StringFile* vertexSource, Assets::StringFile* fragmentSource)
	{
		SetIdentificationFromAssociatedDiskAssetIdentification(vertexSource->GetLabel() + " " + fragmentSource->GetLabel());

		GLint Result = GL_FALSE;
		int32 infoLogLength;

		const GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);

		// Compile Vertex Shader
		RT_XENGINE_LOG_TRACE("Compiling shader : {}", m_vertName);
		char const* VertexSourcePointer = vertexSource->GetFileData().c_str();
		glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
		glCompileShader(VertexShaderID); // Check Vertex Shader
		glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			std::vector<char> VertexShaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(VertexShaderID, infoLogLength, NULL, &VertexShaderErrorMessage[0]);
			RT_XENGINE_LOG_WARN("{}", &VertexShaderErrorMessage[0]);

			return false;
		}
		const GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		// Compile Fragment Shader
		RT_XENGINE_LOG_TRACE("Compiling shader : {}", m_fragName);
		char const* FragmentSourcePointer = fragmentSource->GetFileData().c_str();
		glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
		glCompileShader(FragmentShaderID); // Check Fragment Shader
		glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			std::vector<char> FragmentShaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(FragmentShaderID, infoLogLength, NULL, &FragmentShaderErrorMessage[0]);
			RT_XENGINE_LOG_WARN("{}", &FragmentShaderErrorMessage[0]);

			return false;
		}

		// Link the program
		RT_XENGINE_LOG_DEBUG("Linking program");
		m_programId = glCreateProgram();
		glAttachShader(m_programId, VertexShaderID);
		glAttachShader(m_programId, FragmentShaderID);
		glLinkProgram(m_programId);

		// Check the program
		glGetProgramiv(m_programId, GL_LINK_STATUS, &Result);
		glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			std::vector<char> ProgramErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(m_programId, infoLogLength, NULL, &ProgramErrorMessage[0]);
			RT_XENGINE_LOG_WARN("{}", &ProgramErrorMessage[0]);

			return false;
		}

		glDetachShader(m_programId, VertexShaderID);
		glDetachShader(m_programId, FragmentShaderID);
		glDeleteShader(VertexShaderID);
		glDeleteShader(FragmentShaderID);

		return true;
	}

	void GLShader::SetUniformLocation(const std::string& name)
	{
		m_uniformLocations[name] = glGetUniformLocation(m_programId, name.c_str());
	}

	GLint GLShader::GetUniformLocation(const std::string& name)
	{
		return m_uniformLocations[name];
	}
}
