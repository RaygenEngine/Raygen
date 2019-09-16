#pragma once

#include "renderer/renderers/opengl/GLAsset.h"

#include "GLAD/glad.h"


namespace OpenGL
{
	class GLTexture : public GLAsset
	{
		// bindless
		GLuint64 m_bindlessHandle;
		GLuint m_glId;

	public:
		GLTexture(const fs::path& assocPath)
			: GLAsset(assocPath),
			  m_bindlessHandle(0),
			  m_glId(0)
		{
		}

		bool Load() override;

		friend class GLAssetManager;
	public:
		virtual ~GLTexture();
	
		[[nodiscard]] GLuint GetGLId() const { return m_glId; }
		[[nodiscard]] GLuint64 GetGLBindlessHandle() const { return m_bindlessHandle; }
	};
}
