#pragma once
#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"
#include "asset/assets/ImageAsset.h"

struct SamplerPod
{
	TextureFiltering minFilter{TextureFiltering::LINEAR};
	TextureFiltering magFilter{TextureFiltering::LINEAR};

	TextureWrapping wrapS{TextureWrapping::REPEAT};
	TextureWrapping wrapT{TextureWrapping::REPEAT};
	TextureWrapping wrapR{TextureWrapping::REPEAT};

	ImageAsset* image;
};

