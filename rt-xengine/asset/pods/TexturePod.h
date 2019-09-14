#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"
#include "asset/pods/ImagePod.h"

struct TexturePod : AssetPod
{
	TextureFiltering minFilter{TextureFiltering::LINEAR};
	TextureFiltering magFilter{TextureFiltering::LINEAR};

	TextureWrapping wrapS{TextureWrapping::REPEAT};
	TextureWrapping wrapT{TextureWrapping::REPEAT};
	TextureWrapping wrapR{TextureWrapping::REPEAT};

	ImagePod* image{ nullptr };
};

