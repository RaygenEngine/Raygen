#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "asset/assets/ImageAsset.h"
#include "asset/Asset.h"

#include "GLAD/glad.h"
#include "asset/pods/SamplerPod.h"
#

namespace OpenGL
{
	class GLTexture : public GLAsset
	{
		SamplerPod* m_sampler;
		
		// bindless
		GLuint64 m_bindlessHandle;
		GLuint m_glId;

		GLTexture(SamplerPod* sampler)
			: m_sampler(sampler),
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
