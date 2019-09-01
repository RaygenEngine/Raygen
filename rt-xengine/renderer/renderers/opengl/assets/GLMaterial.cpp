#include "pch.h"

#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "renderer/renderers/opengl/GLRendererBase.h"
#include "renderer/renderers/opengl/GLUtil.h"

namespace Renderer::OpenGL
{
	GLMaterial::GLMaterial(GLRendererBase* renderer, const std::string& name)
		: GLAsset(renderer, name),
		  m_baseColorFactor(1.f, 1.f, 1.f, 1.f),
		  m_emissiveFactor(0.f, 0.f, 0.f),
		  m_metallicFactor(1.f),
		  m_roughnessFactor(1.f),
		  m_normalScale(1.f),
		  m_occlusionStrength(1.f),
		  m_alphaMode(AM_OPAQUE),
		  m_alphaCutoff(0.5f),
		  m_doubleSided(false)
	{
	}

	bool GLMaterial::Load(const Assets::Material& data)
	{
		m_baseColorFactor = data.GetBaseColorFactor();
		m_emissiveFactor = data.GetEmissiveFactor();
		m_metallicFactor = data.GetMetallicFactor();
		m_roughnessFactor = data.GetRoughnessFactor();
		m_normalScale = data.GetNormalScale();
		m_occlusionStrength = data.GetOcclusionStrength();
		m_alphaMode = data.GetAlphaMode();
		m_alphaCutoff = data.GetAlphaCutoff();
		m_doubleSided = data.IsDoubleSided();

		const auto LoadTextureFromSampler = [&](auto& texture, auto& sampler)
		{
			auto text = sampler.GetTexture();
			
			if(text)
				texture = GetGLRenderer()->RequestGLTexture(text, GetGLFiltering(sampler.GetMinFilter()),
					GetGLFiltering(sampler.GetMagFilter()), GetGLWrapping(sampler.GetWrapS()), GetGLWrapping(sampler.GetWrapT()), GetGLWrapping(sampler.GetWrapR()));
		};

		LoadTextureFromSampler(m_baseColorTexture, data.GetBaseColorTextureSampler());
		LoadTextureFromSampler(m_metallicRoughnessTexture, data.GetMetallicRoughnessTextureSampler());
		LoadTextureFromSampler(m_normalTexture, data.GetNormalTextureSampler());
		LoadTextureFromSampler(m_occlusionTexture, data.GetOcclusionTextureSampler());
		LoadTextureFromSampler(m_emissiveTexture, data.GetEmissiveTextureSampler());

		return true;
	}
}
