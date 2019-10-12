#pragma once

#include "reflection/GenMacros.h"
#include "asset/PodHandle.h"
#include "asset/pods/ImagePod.h"

struct TexturePod : public AssetPod {
	REFLECTED_POD(TexturePod)
	{
		REFLECT_VAR(images);
		REFLECT_VAR(minFilter);
		REFLECT_VAR(magFilter);

		REFLECT_VAR(wrapS);
		REFLECT_VAR(wrapT);
		REFLECT_VAR(wrapR);

		REFLECT_VAR(target);
	}
	static void Load(TexturePod* pod, const uri::Uri& path);

	TextureFiltering minFilter{ TextureFiltering::LINEAR };
	TextureFiltering magFilter{ TextureFiltering::LINEAR };

	TextureWrapping wrapS{ TextureWrapping::REPEAT };
	TextureWrapping wrapT{ TextureWrapping::REPEAT };
	TextureWrapping wrapR{ TextureWrapping::REPEAT };

	TextureTarget target{ TextureTarget::TEXTURE_2D };

	std::vector<PodHandle<ImagePod>> images{};
};
