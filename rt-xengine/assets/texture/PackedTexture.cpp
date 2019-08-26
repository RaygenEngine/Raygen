#include "pch.h"

#include "assets/texture/PackedTexture.h"

namespace Assets
{
	PackedTexture::PackedTexture(DiskAssetManager* context, const std::string& path)
		: Texture(context, path)
	{
	}

	bool PackedTexture::Load(Texture* textTargetRChannel, uint32 actualComponents0,
							 Texture* textTargetGChannel, uint32 actualComponents1,
							 Texture* textTargetBChannel, uint32 actualComponents2,
							 Texture* textTargetAChannel, uint32 actualComponents3, DynamicRange dr)
	{
		// helps check stuff about the textures you want to pack
		std::vector<std::tuple<uint32, Texture*, uint32>> textures;

		if (textTargetRChannel)
			textures.emplace_back(CT_RED, textTargetRChannel, actualComponents0);
		else if (actualComponents0 != 0)
			RT_XENGINE_LOG_ERROR("Null texture with channel target RED and actual components value != 0, cache miss danger!");

		if(textTargetGChannel)
			textures.emplace_back(CT_GREEN, textTargetGChannel, actualComponents1);
		else if (actualComponents1 != 0)
			RT_XENGINE_LOG_ERROR("Null texture with channel target GREEN and actual components value != 0, cache miss danger!");

		if (textTargetBChannel)
			textures.emplace_back(CT_BLUE, textTargetBChannel, actualComponents2);
		else if (actualComponents2 != 0)
			RT_XENGINE_LOG_ERROR("Null texture with channel target BLUE and actual components value != 0, cache miss danger!");

		if (textTargetAChannel)
			textures.emplace_back(CT_ALPHA, textTargetAChannel, actualComponents3);
		else if (actualComponents3 != 0)
			RT_XENGINE_LOG_ERROR("Null texture with channel target ALPHA and actual components value != 0, cache miss danger!");

		m_dynamicRange = dr;
		m_components = 4; // actual components of packed always 4?

		for(const auto& t : textures)
		{
			if(m_dynamicRange != std::get<1>(t)->GetType())
			{
				RT_XENGINE_LOG_ERROR("Cannot pack different typed textures together (hdr with ldr)");
				return false;
			}

			// not first or default texture, difference in width or height, trying to pack different sized textures 
			if (std::get<1>(t)->GetWidth() > 1 && m_width > 1 && m_width != std::get<1>(t)->GetWidth() ||
				std::get<1>(t)->GetHeight() > 1 && m_height > 1 && m_height != std::get<1>(t)->GetHeight())
			{
				RT_XENGINE_LOG_ERROR("Trying to pack different sized textures!");
				return false;
			}

			if (m_width < std::get<1>(t)->GetWidth())
				m_width = std::get<1>(t)->GetWidth();

			if (m_height < std::get<1>(t)->GetHeight())
				m_height = std::get<1>(t)->GetHeight();
		}

		switch (m_dynamicRange)
		{
			case DR_LOW:
			
				ReserveTextureDataMemory(sizeof(byte) * 4 * m_width * m_height);
				for (const auto& t : textures)
				{
					if (!LoadChannels<byte>(std::get<0>(t), std::get<1>(t), std::get<2>(t)))
						return false;
				}
				break;

			case DR_HIGH:
				ReserveTextureDataMemory(sizeof(float) * 4 * m_width * m_height);
				for (const auto& t : textures)
				{
					if (!LoadChannels<float>(std::get<0>(t), std::get<1>(t), std::get<2>(t)))
						return false;
				}
				break;
		}
		
		return true;
	}

	template<typename T>
	bool PackedTexture::LoadChannels(uint32 targetChannel, Texture* text, uint32 actualComponents)
	{
		const auto fitCheck = targetChannel + (actualComponents >= 4 ? 3 : actualComponents);
		// will this packing fit?
		if (fitCheck > 4)
		{
			RT_XENGINE_LOG_ERROR("Cannot fit packing, target channel: {}, actual components: {}", targetChannel, actualComponents);
			return false;
		}

		auto* packedData = reinterpret_cast<T*>(m_data);
		auto* textData = reinterpret_cast<T*>(text->GetData());

		bool isDefault = false;
		// if from default texture
		if (text->GetHeight() * text->GetWidth() == 1)
			isDefault = true;

		for(auto i = 0u; i < 4 * m_width * m_height; i+= 4)
		{
			const auto j = isDefault ? 0 : i;

			switch (actualComponents)
			{
			case 1: // RRR1 -> target = R
				packedData[i + targetChannel + CT_RED] = textData[j + CT_RED];
				break;
				
			case 2: // RRRG -> target = R | target + 1 = G
				packedData[i + targetChannel + CT_RED] = textData[j + CT_RED];
				packedData[i + targetChannel + CT_GREEN] = textData[j + CT_ALPHA];
				break;

			case 3: // RGB1 -> target = R | target + 1 = G |  target + 2 = B
			case 4: // RGBA -> target = R | target + 1 = G |  target + 2 = B (lose the Alpha - note: otherwise no point in packing this texture)
				packedData[i + targetChannel + CT_RED] = textData[j + CT_RED];
				packedData[i + targetChannel + CT_GREEN] = textData[j + CT_GREEN];
				packedData[i + targetChannel + CT_BLUE] = textData[j + CT_BLUE];
				break;

			default:
				RT_XENGINE_LOG_ERROR("Actual components must be <= 4");
				return false;
			}
		}
		// all good
		return true;
	}
}
