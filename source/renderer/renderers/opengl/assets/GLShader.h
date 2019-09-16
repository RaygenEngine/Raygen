#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "asset/loaders/ShaderLoader.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	class GLShader : public GLAsset
	{
		GLuint m_glId;

		// temporary
		std::unordered_map<std::string, GLint> m_uniformLocations;

	public:
		GLShader(const fs::path& assocPath)
			: GLAsset(assocPath),
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
