#pragma once

#include "renderer/renderers/opengl/GLAsset.h"

#include "GLAD/glad.h"


namespace OpenGL
{
	class GLTexture : public GLAsset
	{
		// bindless
		GLuint64 m_bindlessId;
		GLuint m_id;

	public:
		GLTexture(const fs::path& assocPath)
			: GLAsset(assocPath),
			  m_bindlessId(0),
			  m_id(0)
		{
		}
		virtual ~GLTexture();

		bool Load() override;

		[[nodiscard]] GLuint GetId() const { return m_id; }
		[[nodiscard]] GLuint64 GetBindlessId() const { return m_bindlessId; }
	};
}
