#pragma once

#include "assets/FileAsset.h"

// TODO: expand (bpp, bpc, load in the file format, etc.)

// When loading a texture, we force the following conversions at rgba
// image_cmp, forced_cmp
//(1, 4) { dest[0] = dest[1] = dest[2] = src[0], dest[3] = 255; } break; R -> RRR1
//(2, 4) { dest[0] = dest[1] = dest[2] = src[0], dest[3] = src[1]; } break; RG -> RRRG
//(3, 4) { dest[0] = src[0], dest[1] = src[1], dest[2] = src[2], dest[3] = 255; } break; RGB -> RGB1

// rgba T(float, byte, short - w/e stb supports) texture
class Texture : public FileAsset
{
protected:
	uint32 m_width;
	uint32 m_height;
	// actual textures components -> final texture has RGBA, but this value gives insight to how the data is stored
	// however same texture types (e.g heightmaps) may have different actual components (1-grayscale, 4-rgba with r=g=b) based on the image's properties
	// therefore this value isn't very useful 
	uint32 m_components;

	// use malloc and free
	void* m_data;

	bool m_hdr;

public:
	Texture(const std::string& path)
		: FileAsset(path),
		  m_width(0),
		  m_height(0),
		  m_components(0),
		  m_data(nullptr),
		  m_hdr(false) {}
	virtual ~Texture();

	bool Load(const std::string& path);
	void Clear() override;

	[[nodiscard]] uint32 GetWidth() const { return m_width; }
	[[nodiscard]] uint32 GetHeight() const { return m_height; }
	[[nodiscard]] uint32 GetComponents() const { return m_components; }
	[[nodiscard]] void* GetData() const { return m_data; }

	[[nodiscard]] bool IsHdr() const { return m_hdr; }

	// creates default rgba texture
	// must be of type glm::Tvec
	template<typename T>
	static std::unique_ptr<Texture> CreateDefaultTexture(const T& defaultValue, uint32 width,
		uint32 height, const std::string& defaultName = "DefaultTexture")

	{
		using value_type = typename T::value_type;
		
		std::unique_ptr<Texture> texture = std::make_unique<Texture>(defaultName);

		texture->m_width = width;
		texture->m_height = height;
		texture->m_components = T::length();
		texture->m_hdr = std::is_floating_point_v<value_type>;

		LOG_INFO("Creating default texture, name:{}, width: {}, height: {}, components: {}, hdr: {}", defaultName, width, height, texture->m_components, texture->m_hdr);
		
		const auto size = 4 * texture->m_width * texture->m_height;

		texture->m_data = malloc(sizeof(value_type) * size);
		auto textureBuffer = static_cast<value_type*>(texture->m_data);
		
		for (auto i = 0u; i < size; i += 4)
		{
			switch (texture->m_components)
			{

			case 1: // R -> RRR1
				textureBuffer[i] = defaultValue[0];
				textureBuffer[i + 1] = defaultValue[0];
				textureBuffer[i + 2] = defaultValue[0];
				textureBuffer[i + 3] = static_cast<value_type>(1);
				break;

			case 2: // RG -> RRRG (R is value and G is Alpha)
				textureBuffer[i] = defaultValue[0];
				textureBuffer[i + 1] = defaultValue[0];
				textureBuffer[i + 2] = defaultValue[0];
				textureBuffer[i + 3] = defaultValue[1];
				break;

			case 3: // RGB -> RGB1
				textureBuffer[i] = defaultValue[0];
				textureBuffer[i + 1] = defaultValue[1];
				textureBuffer[i + 2] = defaultValue[2];
				textureBuffer[i + 3] = static_cast<value_type>(1);
				break;

			case 4: // RGBA -> RGBA
				textureBuffer[i] = defaultValue[0];
				textureBuffer[i + 1] = defaultValue[1];
				textureBuffer[i + 2] = defaultValue[2];
				textureBuffer[i + 3] = defaultValue[3];
				break;
			}
		}
		
		texture->MarkLoaded();

		return texture;
	}
	
	void ToString(std::ostream& os) const override { os << "asset-type: Texture, name: " << m_fileName << ", type: " << (m_hdr ? "hdr" : "ldr"); }
};

