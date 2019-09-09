#pragma once

#include "assets/texture/Texture.h"

// pack textures as disk assets and not as gpu assets - avoid creating packed textures for each gpu context
class PackedTexture : public Texture
{
	// caching
	std::vector<std::shared_ptr<Texture>> m_parts;

	// inits based on first input
	bool m_init;
	// 0 - 4 (data is packed sequentially)
	uint32 m_availableChannel;

public:
	PackedTexture(const std::string& path)
		: Texture(path),
	      m_init(false),
	      m_availableChannel(0) {}

	~PackedTexture() = default;
	
	bool LoadTextureAtChannelTarget(std::shared_ptr<Texture>& texture, TextureChannel srcChannels, bool cacheOriginal = false);
};

