#ifndef GLCUBEMAP_H
#define GLCUBEMAP_H

#include "renderer/renderers/opengl/GLAsset.h"

#include "glad/glad.h"

namespace Renderer::OpenGL
{
	// TODO: This asset has not been tested in a renderer, if it doesn't work please implement it correctly, if it works, remove this comment
	class GLCubeMap : public GLAsset
	{
		GLuint m_cubeTextureId;

	public:
		GLCubeMap(GLRendererBase* renderer);
		~GLCubeMap();

		bool Load(Assets::CubeMap* data, GLint wrapFlag, bool mipMapping);

		GLuint GetGLHandle() const { return m_cubeTextureId; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLCubeTexture, name: " << m_associatedDescription; }
	};
}

#endif // GLCUBEMAP_H
