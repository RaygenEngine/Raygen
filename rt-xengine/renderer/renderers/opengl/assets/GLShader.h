#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "assets/other/utf8/StringFile.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	class GLShader : public GLAsset
	{
		GLuint m_glId;

		// temporary
		std::unordered_map<std::string, GLint> m_uniformLocations;

	public:
		GLShader(const std::string& name)
			: GLAsset(name),
			  m_glId(0) {}
		~GLShader();

		bool Load(StringFile* vertexSource, StringFile* fragmentSource);

		GLuint GetGLHandle() const { return m_glId; }

		void SetUniformLocation(const std::string& name);
		GLint GetUniformLocation(const std::string& name);

		void ToString(std::ostream& os) const override { os << "asset-type: GLShader, name: " << m_name; }
	};

}
