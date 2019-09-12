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

	public:
		GLTexture(SamplerPod* sampler)
			: GLAsset(sampler->image),
			m_sampler(sampler),
			  m_bindlessHandle(0),
			  m_glId(0)
		{
		}
		~GLTexture();
	
		[[nodiscard]] GLuint GetGLId() const { return m_glId; }
		[[nodiscard]] GLuint64 GetGLBindlessHandle() const { return m_bindlessHandle; }

		bool Load() override;
		void Unload() override;
	};
}
