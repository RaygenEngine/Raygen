#ifndef GLSHADER_H
#define GLSHADER_H

#include "renderer/renderers/opengl/GLAsset.h"

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
		GLShader(GLRendererBase* renderer);
		~GLShader();

		bool Load(Assets::StringFile* vertexSource, Assets::StringFile* fragmentSource);

		GLuint GetGLHandle() const { return m_programId; }

		void SetUniformLocation(const std::string& name);
		GLint GetUniformLocation(const std::string& name);

		void ToString(std::ostream& os) const override { os << "asset-type: GLShader, vert name: " << m_vertName << " frag name : " << m_fragName; }
	};

}

#endif // GLSHADER_H
