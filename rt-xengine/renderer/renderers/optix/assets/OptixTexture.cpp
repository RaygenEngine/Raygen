#include "pch.h"
#include "OptixTexture.h"

namespace Renderer::Optix
{
	OptixTexture::OptixTexture(OptixRendererBase* renderer)
		: OptixAsset(renderer)
	{
	}

	bool OptixTexture::Load(Assets::Texture* data)
	{
		SetIdentificationFromAssociatedDiskAssetIdentification(data->GetLabel());

		m_handle = GetOptixContext()->createTextureSampler();
		m_handle->setWrapMode(0, RT_WRAP_REPEAT);

		m_handle->setWrapMode(1, RT_WRAP_REPEAT);

		m_handle->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
		m_handle->setReadMode(RT_TEXTURE_READ_NORMALIZED_FLOAT);

		m_handle->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);

		m_handle->setMaxAnisotropy(1.0f);
		m_handle->setMipLevelCount(1u);
		m_handle->setArraySize(1u);

		optix::Buffer assocBuffer;


		switch (data->GetType())
		{
			// considered hdr - sampler doesn't normalize
		case Assets::DR_HIGH:
		{
			assocBuffer = GetOptixContext()->createBuffer(RT_BUFFER_INPUT | RT_BUFFER_DISCARD_HOST_MEMORY, RT_FORMAT_FLOAT4);
			assocBuffer->setSize(data->GetWidth(), data->GetHeight());

			memcpy(assocBuffer->map(), data->GetData(), data->GetWidth() * data->GetHeight() * sizeof(float) * 4);
			break;
		}
		case Assets::DR_LOW:
		default:
			assocBuffer = GetOptixContext()->createBuffer(RT_BUFFER_INPUT | RT_BUFFER_DISCARD_HOST_MEMORY, RT_FORMAT_UNSIGNED_BYTE4);
			assocBuffer->setSize(data->GetWidth(), data->GetHeight());

			memcpy(assocBuffer->map(), data->GetData(), data->GetWidth() * data->GetHeight() * sizeof(byte) * 4);
			break;
		}

		assocBuffer->unmap();
		
		m_handle->setBuffer(assocBuffer);

		return true;
	}
}
