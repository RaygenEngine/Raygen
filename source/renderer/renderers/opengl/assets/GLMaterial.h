#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "asset/pods/MaterialPod.h"

namespace OpenGL
{
	struct GLMaterial : GLAsset
	{
		// RGB: Albedo A: Opacity
		GLTexture* baseColorTexture{ nullptr };
		// R: *occlusion, G: Roughness, B: Metal, A: empty
		GLTexture* metallicRoughnessTexture{ nullptr };
		// R: occlusion, accessing other channels may give wrong info
		GLTexture* occlusionTexture{ nullptr };
		GLTexture* normalTexture{ nullptr };
		GLTexture* emissiveTexture{ nullptr };
		
		PodHandle<MaterialPod> m_materialPod;

		GLMaterial(const fs::path& assocPath)
			: GLAsset(assocPath)
		{
		}

	protected:
		bool Load() override;
	};

}
