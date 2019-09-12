#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "asset/assets/ImageAsset.h"
#include "asset/Asset.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	class GLTexture : public GLAsset
	{
		ImageAsset* m_textureData;
		
		// bindless
		GLuint64 m_bindlessHandle;
		GLuint m_glId;

	public:
		GLTexture(ImageAsset* textureData)
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
