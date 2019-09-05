#include "pch.h"

#include "assets/texture/Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

namespace Assets
{
	// When loading a texture, we force the following conversions at rgba
	// image_cmp, forced_cmp
	//(1, 4) { dest[0] = dest[1] = dest[2] = src[0], dest[3] = 255; } break; R -> RRR1
	//(2, 4) { dest[0] = dest[1] = dest[2] = src[0], dest[3] = src[1]; } break; RG -> RRRG
	//(3, 4) { dest[0] = src[0], dest[1] = src[1], dest[2] = src[2], dest[3] = 255; } break; RGB -> RGB1
	template<typename T>
	inline void CopyValueTypedToTextureBuffer(T* textureBuffer, T* valueBuffer, uint32 size, uint32 components, T extra)
	{
		for (auto i = 0u; i < size; i += 4)
		{
			switch (components)
			{

			case 1: // R -> RRR1
				textureBuffer[i] = valueBuffer[0];
				textureBuffer[i + 1] = valueBuffer[0];
				textureBuffer[i + 2] = valueBuffer[0];
				textureBuffer[i + 3] = extra;
				break;

			case 2: // RG -> RRRG (R is value and G is Alpha)
				textureBuffer[i] = valueBuffer[0];
				textureBuffer[i + 1] = valueBuffer[0];
				textureBuffer[i + 2] = valueBuffer[0];
				textureBuffer[i + 3] = valueBuffer[1];
				break;

			case 3: // RGB -> RGB1
				textureBuffer[i] = valueBuffer[0];
				textureBuffer[i + 1] = valueBuffer[1];
				textureBuffer[i + 2] = valueBuffer[2];
				textureBuffer[i + 3] = extra;
				break;

			case 4: // RGBA -> RGBA
				textureBuffer[i] = valueBuffer[0];
				textureBuffer[i + 1] = valueBuffer[1];
				textureBuffer[i + 2] = valueBuffer[2];
				textureBuffer[i + 3] = valueBuffer[3];
				break;
			}
		}
	}
	
	Texture::Texture(EngineObject* pObject, const std::string& path)
		: DiskAsset(pObject, path),
		  m_width(0),
		  m_height(0),
		  m_components(0),
		  m_dynamicRange(),
	      m_data(nullptr)
	{
	}

	Texture::~Texture()
	{
		Unload();
	}

	bool Texture::Load(const std::string& path, DynamicRange dr, bool flipVertically)
	{
		int32 width;
		int32 height;
		int32 components;

		void* imageData = nullptr;

		m_dynamicRange = dr;


		stbi_set_flip_vertically_on_load(flipVertically);

		switch (m_dynamicRange)
		{
		case DynamicRange::LOW:
			imageData = stbi_load(path.c_str(), &width, &height, &components, STBI_rgb_alpha);
			break;

		case DynamicRange::HIGH:
			imageData = stbi_loadf(path.c_str(), &width, &height, &components, STBI_rgb_alpha);
			break;

		default:
			RT_XENGINE_LOG_ERROR("Texel data type should be either DR_LOW or DR_HIGH");
			return false;
		}

		// return to previous state
		stbi_set_flip_vertically_on_load(!flipVertically);

		if (!imageData || (width == 0) || (height == 0))
		{
			RT_XENGINE_LOG_WARN("Texture loading failed --> filepath: {}, data_empty: {} width: {} height: {}", path,
				static_cast<bool>(imageData), width, height);

			return false;
		}

		m_width = width;
		m_height = height;
		m_components = components;

		m_data = imageData;

		return true;
	}

	void Texture::Clear()
	{
		stbi_image_free(m_data);
	}

	std::unique_ptr<Texture> Texture::CreateDefaultTexture(EngineObject* pObject, void* texelValue, uint32 width,
		uint32 height, uint32 components, DynamicRange dr, const std::string& defaultName)
	{
		RT_XENGINE_LOG_DEBUG("Creating default texture, name:{}, width: {}, height: {}, components: {}, type: {}", defaultName, width, height, components, TexelEnumToString(dr));

		RT_XENGINE_ASSERT(components <= 4, "Couldn't create default texture, name:{}", defaultName);

		std::unique_ptr<Texture> texture = std::make_unique<Texture>(pObject, defaultName);

		texture->m_width = width;
		texture->m_height = height;
		texture->m_components = components;
		texture->m_dynamicRange = dr;


		const auto size = 4 * texture->m_width*texture->m_height;

		switch (texture->m_dynamicRange)
		{

		case DynamicRange::LOW:
			texture->ReserveTextureDataMemory(sizeof(byte) * size);
			CopyValueTypedToTextureBuffer(static_cast<byte*>(texture->m_data), static_cast<byte*>(texelValue), size, components, static_cast<byte>(255));
			break;

		case DynamicRange::HIGH:
			texture->ReserveTextureDataMemory(sizeof(float) * size);
			CopyValueTypedToTextureBuffer(static_cast<float*>(texture->m_data), static_cast<float*>(texelValue), size, components, 1.0f);
			break;
		}

		texture->MarkLoaded();

		return texture;
	}

	void Texture::ReserveTextureDataMemory(uint32 size)
	{
		m_data = STBI_MALLOC(size);
	}
}
