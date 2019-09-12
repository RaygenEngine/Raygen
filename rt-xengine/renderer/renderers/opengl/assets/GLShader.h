#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "assets/other/utf8/MultiStringFileAsset.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	class GLShader : public GLAsset
	{
		MultiStringFileAsset* m_sources;
		
		GLuint m_glId;

		// temporary
		std::unordered_map<std::string, GLint> m_uniformLocations;

	public:
		GLShader(MultiStringFileAsset* sources)
			: GLAsset(sources),
			  m_sources(sources),
			  m_glId(0) {}

		~GLShader();

		[[nodiscard]] GLuint GetGLHandle() const { return m_glId; }

		void SetUniformLocation(const std::string& name);
		GLint GetUniformLocation(const std::string& name);

	protected:
		bool Load() override;
		void Unload() override;
	};

}
