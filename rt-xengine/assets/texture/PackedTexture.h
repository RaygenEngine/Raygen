#pragma once

#include "assets/texture/Texture.h"

namespace Assets
{

	// pack textures as disk assets and not as gpu assets - avoid creating packed textures for each gpu context
	class PackedTexture : public Texture
	{

		template<typename T>
		bool LoadChannels(uint32 targetChannel, Texture* text, uint32 actualComponents);

	public:
		PackedTexture(EngineObject* pObject, const std::string& path);
		~PackedTexture() = default;

		// pack at most 4 textures into a single one, copying channels based on actual texture components,
		// if less than 4 textures used, pass nullptr and actual components value 0, if not enough space for packing, packed texture will not be loaded
		bool Load(Texture* textTargetRChannel, uint32 actualComponents0,
			Texture* textTargetGChannel, uint32 actualComponents1,
			Texture* textTargetBChannel, uint32 actualComponents2,
			Texture* textTargetAChannel, uint32 actualComponents3, DynamicRange dr);
	};
}
