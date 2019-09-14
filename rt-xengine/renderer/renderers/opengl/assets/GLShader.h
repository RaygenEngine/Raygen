#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "asset/assets/ShaderAsset.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	class GLShader : public GLAsset
	{
		ShaderAsset* m_sources;
		
		GLuint m_glId;

		// temporary
		std::unordered_map<std::string, GLint> m_uniformLocations;

	
		GLShader(ShaderAsset* sources)
			: m_sources(sources),
			  m_glId(0) {}

		bool Load() override;
		
		friend class GLAssetManager;
	public:
		virtual ~GLShader();

		[[nodiscard]] GLuint GetGLHandle() const { return m_glId; }

		void SetUniformLocation(const std::string& name);
		GLint GetUniformLocation(const std::string& name);
	};

}
