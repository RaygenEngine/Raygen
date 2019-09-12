#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "assets/texture/TextureAsset.h"
#include "assets/Asset.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	class GLTexture : public GLAsset
	{
		TextureAsset* m_textureData;
		
		// bindless
		GLuint64 m_bindlessHandle;
		GLuint m_glId;

	public:
		GLTexture(TextureAsset* textureData)
			: GLAsset(textureData),
			  m_textureData(textureData),
			  m_bindlessHandle(0),
			  m_glId(0)
		{
		}
		~GLTexture();
	
		[[nodiscard]] GLuint GetGLId() const { return m_glId; }
		[[nodiscard]] GLuint64 GetGLBindlessHandle() const { return m_bindlessHandle; }

	protected:
		bool Load() override;
		void Unload() override;
	};
}
