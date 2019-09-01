#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "assets/texture/Texture.h"

#include "GLAD/glad.h"

namespace Renderer::OpenGL
{
	class GLTexture : public GLAsset
	{
		// bindless
		GLuint64 m_bindlessHandle;
		GLuint m_Id;

	public:
		GLTexture(GLAssetManager* glAssetManager, const std::string& name);
		~GLTexture();

		int32 m_texCoordIndex;
		
		bool Load(Assets::Texture* data, GLint minFilter, GLint magFilter, GLint wrapS, GLint wrapT, GLint wrapR);

		GLuint GetGLId() const { return m_Id; }
		GLuint64 GetGLBindlessHandle() const { return m_bindlessHandle; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLTexture, name: " << m_name; }
	};
}
