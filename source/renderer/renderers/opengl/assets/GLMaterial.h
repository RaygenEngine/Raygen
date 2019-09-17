#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "asset/pods/MaterialPod.h"

namespace OpenGL
{
	struct GLMaterial : GLAsset
	{
		// RGB: Albedo A: Opacity
		GLTexture* baseColorTexture;
		// R: *occlusion, G: Roughness, B: Metal, A: empty
		GLTexture* metallicRoughnessTexture;
		// R: occlusion, accessing other channels may give wrong info
		GLTexture* occlusionTexture;
		GLTexture* normalTexture;
		GLTexture* emissiveTexture;
		
		PodHandle<MaterialPod> m_materialPod;

		GLMaterial(const fs::path& assocPath)
			: GLAsset(assocPath),
			  baseColorTexture(nullptr),
			  metallicRoughnessTexture(nullptr),
		      occlusionTexture(nullptr),
			  normalTexture(nullptr),
			  emissiveTexture(nullptr)
		{
		}

	protected:
		bool Load() override;
	};

}
