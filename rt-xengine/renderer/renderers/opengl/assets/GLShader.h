#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "assets/other/utf8/StringFile.h"

#include "GLAD/glad.h"

namespace Renderer::OpenGL
{

	class GLShader : public GLAsset
	{
		GLuint m_programId;
		std::string m_vertName;
		std::string m_fragName;

		// temporary
		std::unordered_map<std::string, GLint> m_uniformLocations;

	public:
		GLShader(GLAssetManager* glAssetManager, const std::string& name);
		~GLShader();

		bool Load(Assets::StringFile* vertexSource, Assets::StringFile* fragmentSource);

		GLuint GetGLHandle() const { return m_programId; }

		void SetUniformLocation(const std::string& name);
		GLint GetUniformLocation(const std::string& name);

		void ToString(std::ostream& os) const override { os << "asset-type: GLShader, name: " << m_name; }
	};

}
