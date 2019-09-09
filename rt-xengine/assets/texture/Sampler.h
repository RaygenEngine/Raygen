#pragma once

#include "assets/texture/Texture.h"

struct Sampler : Object
{
	std::shared_ptr<Texture> texture = nullptr;

	TextureFiltering minFilter = TextureFiltering::LINEAR;
	TextureFiltering magFilter = TextureFiltering::LINEAR;

	TextureWrapping wrapS = TextureWrapping::REPEAT;
	TextureWrapping wrapT = TextureWrapping::REPEAT;
	TextureWrapping wrapR = TextureWrapping::REPEAT;

	int32 texCoordIndex = 0;
};