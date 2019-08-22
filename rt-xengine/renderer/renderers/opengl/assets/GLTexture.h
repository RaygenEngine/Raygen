#ifndef GLTEXTURE_H
#define GLTEXTURE_H

#include "renderer/renderers/opengl/GLAsset.h"

#include "assets/texture/Texture.h"

#include "glad/glad.h"


namespace Renderer::OpenGL
{
	class GLTexture : public GLAsset
	{
		GLuint m_textureId;

	public:
		GLTexture(GLRendererBase* renderer);
		~GLTexture();

		bool Load(Assets::Texture* data, GLint wrapFlag, bool mipMapping);

		GLuint GetGLHandle() const { return m_textureId; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLTexture, name: " << m_associatedDescription; }
	};
}

#endif // GLTEXTURE_H
