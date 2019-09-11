#include "pch.h"

#include "assets/texture/PackedTexture.h"
#include "assets/AssetManager.h"

namespace
{

	template<typename T>
	void LoadDataToChannels(T* srcData, TextureChannel srcChannels, T* dstData, int32 size, int32 components, int32 availableChannel)
	{
		for (int32 i = 0; i < size; i += components)
		{
			auto targetChannel = availableChannel;

			if (srcChannels & TC_RED)
			{
				dstData[i + targetChannel++] = srcData[i];
			}

			if (srcChannels & TC_GREEN)
			{
				dstData[i + targetChannel++] = srcData[i + 1];
			}

			if (srcChannels & TC_BLUE)
			{
				dstData[i + targetChannel++] = srcData[i + 2];
			}

			if (srcChannels & TC_ALPHA)
			{
				dstData[i + targetChannel++] = srcData[i + 3];
			}
		}
	}

}

bool PackedTexture::LoadTextureAtChannelTarget(std::shared_ptr<Texture>& texture, TextureChannel srcChannels, bool cacheOriginal)
{
	const auto srcChannelCount = utl::CountSetBits(srcChannels);
	
	if(srcChannelCount + m_availableChannel > 4)
	{
		LOG_ERROR("Not enough channels to store source channels");
		return false;
	}

	
	// init based on the first texture 
	if(!m_init)
	{
		//m_dynamicRange = texture->GetDynamicRange();
		// packed texture 4 components
		//m_components = 4;

		//m_width = texture->GetWidth();
		//m_height = texture->GetHeight();

		//switch (m_dynamicRange)
		//{
		//case DynamicRange::LOW:
		//	ReserveTextureDataMemory(sizeof(byte) * m_components * m_width * m_height);
		//	break;

		//case DynamicRange::HIGH:
		//	ReserveTextureDataMemory(sizeof(float) * m_components * m_width * m_height);
		//	break;
		//}

		m_init = true;
	}
	
	else
	{
		//if(m_dynamicRange != texture->GetDynamicRange())
		//{
		//	LOG_ERROR("Missmatched dynamic range in packed texture");
		//	return false;
		//}

		/*if (m_width != texture->GetWidth())
		{
			LOG_ERROR("Missmatched width in packed texture");
			return false;
		}

		if (m_height != texture->GetHeight())
		{
			LOG_ERROR("Missmatched height in packed texture");
			return false;
		}*/
	}
	

	//switch (m_dynamicRange)
	//{
	//case DynamicRange::LOW:
	//	LoadDataToChannels<byte>(reinterpret_cast<byte*>(texture->GetData()), srcChannels, reinterpret_cast<byte*>(m_data),
	//		m_width * m_height * m_components, m_components, m_availableChannel);
	//	break;

	//case DynamicRange::HIGH:
	//	LoadDataToChannels<float>(reinterpret_cast<float*>(texture->GetData()), srcChannels, reinterpret_cast<float*>(m_data),
	//		m_width * m_height * m_components, m_components, m_availableChannel);
	//	break;
	//}

	m_availableChannel += srcChannelCount;

	if (cacheOriginal)
		m_parts.emplace_back(texture);
	
	return true;
}

