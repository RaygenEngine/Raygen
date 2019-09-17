#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "asset/loaders/ShaderLoader.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	class GLShader : public GLAsset
	{
		GLuint m_id;

		// temporary
		std::unordered_map<std::string, GLint> m_uniformLocations;

	public:
		GLShader(const fs::path& assocPath)
			: GLAsset(assocPath),
			  m_id(0) {}
		virtual ~GLShader();
		
		bool Load() override;

		[[nodiscard]] GLuint GetId() const { return m_id; }

		GLint operator[](const std::string& uniformName) const;
		void operator+=(const std::string& uniformName);
	};

}
