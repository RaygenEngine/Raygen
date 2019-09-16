#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "asset/loaders/CubemapLoader.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	// TODO: This asset has not been tested in a renderer, if it doesn't work please implement it correctly, if it works, remove this comment
	class GLCubeMap : public GLAsset
	{
		// bindless
		GLuint64 m_bindlessHandle;
		GLuint m_glId;

	public:
		GLCubeMap(const fs::path& assocPath)
			: GLAsset(assocPath),
		      m_bindlessHandle(0),
			  m_glId(0) {}

		bool Load() override;
		friend class GLAssetManager;
	public:
		~GLCubeMap();

		[[nodiscard]] GLuint GetGLId() const { return m_glId; }
		[[nodiscard]] GLuint64 GetGLBindlessHandle() const { return m_bindlessHandle; }

	};
}
