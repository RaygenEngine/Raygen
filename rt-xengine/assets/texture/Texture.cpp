#include "pch.h"

#include "assets/texture/Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

Texture::~Texture()
{
	Unload();
}

bool Texture::Load(const std::string& path)
{
	int32 width;
	int32 height;
	int32 components;

	void* imageData = nullptr;

	m_hdr = stbi_is_hdr(path.c_str()) == 1;

	if(!m_hdr)
		imageData = stbi_load(path.c_str(), &width, &height, &components, STBI_rgb_alpha);
	else
		imageData = stbi_loadf(path.c_str(), &width, &height, &components, STBI_rgb_alpha);
	
	if (!imageData || (width == 0) || (height == 0))
	{
		LOG_WARN("Texture loading failed, filepath: {}, data_empty: {} width: {} height: {}", path,
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
	free(m_data);
}
