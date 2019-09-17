#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "asset/loaders/ShaderLoader.h"

#include "glad/glad.h"

namespace OpenGL
{
	struct GLShader : GLAsset
	{
		GLuint id;
		
		GLShader(const fs::path& assocPath)
			: GLAsset(assocPath) {}
		virtual ~GLShader();
		
		GLint operator[](const std::string& uniformName) const;
		void operator+=(const std::string& uniformName);

	protected:
		// temporary
		std::unordered_map<std::string, GLint> uniformLocations;
		bool Load() override;
	};

}
