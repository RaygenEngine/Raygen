#include "pch.h"
#include "OptixCubeMap.h"

namespace Renderer::Optix
{
	OptixCubeMap::OptixCubeMap(OptixRendererBase* renderer)
		: OptixAsset(renderer)
	{
	}

	bool OptixCubeMap::Load(Assets::CubeMap* data)
	{
		SetIdentificationFromAssociatedDiskAssetIdentification(data->GetLabel());

		m_handle = GetOptixContext()->createTextureSampler();
		// seamless
		m_handle->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE);
		m_handle->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE);

		m_handle->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
		m_handle->setReadMode(RT_TEXTURE_READ_NORMALIZED_FLOAT);

		m_handle->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);

		optix::Buffer assocBuffer;

		switch (data->GetType())
		{
			// considered hdr - sampler doesn't normalize
		case Assets::DR_HIGH:
		{
			assocBuffer = GetOptixContext()->createBuffer(RT_BUFFER_INPUT | RT_BUFFER_CUBEMAP | RT_BUFFER_DISCARD_HOST_MEMORY, RT_FORMAT_FLOAT4);
			assocBuffer->setSize(data->GetWidth(), data->GetHeight(), Assets::CMF_COUNT);

			float* bufferData = nullptr;

			for (auto i = 0; i < Assets::CMF_COUNT; ++i)
			{
				if (i == 0)
					bufferData = static_cast<float*>(assocBuffer->map());
				else
					bufferData += data->GetWidth() * data->GetHeight() * sizeof(float) * 4;

				memcpy(bufferData, data->GetFace(static_cast<Assets::CUBE_MAP_FACE>(i))->GetData(),
					data->GetWidth() * data->GetHeight() * sizeof(float) * 4);
			}

			break;
		}
		case Assets::DR_LOW:
		default:
			assocBuffer = GetOptixContext()->createBuffer(RT_BUFFER_INPUT | RT_BUFFER_CUBEMAP | RT_BUFFER_DISCARD_HOST_MEMORY, RT_FORMAT_UNSIGNED_BYTE4);
			assocBuffer->setSize(data->GetWidth(), data->GetHeight(), Assets::CMF_COUNT);

			byte* bufferData = nullptr;

			for (auto i = 0; i < Assets::CMF_COUNT; ++i)
			{
				if (i == 0)
					bufferData = static_cast<byte*>(assocBuffer->map());
				else 
					bufferData += data->GetWidth() * data->GetHeight() * sizeof(byte) * 4;

				memcpy(bufferData, data->GetFace(static_cast<Assets::CUBE_MAP_FACE>(i))->GetData(),
					data->GetWidth() * data->GetHeight() * sizeof(byte) * 4);
			}

			break;
		}

		assocBuffer->unmap();

		m_handle->setBuffer(assocBuffer);

		return true;
	}
}
