#ifndef TEXTURE_H
#define TEXTURE_H



#include "assets/DiskAsset.h"
#include <string>
#include <memory>

namespace Assets
{
	// Low dynamic range textures are loaded as byte
	// High dynamic range textures are loaded as float32
	enum DYNAMIC_RANGE
	{
		DR_LOW = 0,
		DR_HIGH
	};

	inline const char* TexelEnumToString(DYNAMIC_RANGE dr)
	{
		switch(dr) 
		{
			case DR_LOW: return "LDR";
			case DR_HIGH: return "HDR";
			default:  return "invalid";
		}
	}


	// When loading a texture, we force the following conversions at rgba
	// image_cmp, forced_cmp
	//(1, 4) { dest[0] = dest[1] = dest[2] = src[0], dest[3] = 255; } break; R -> RRR1
	//(2, 4) { dest[0] = dest[1] = dest[2] = src[0], dest[3] = src[1]; } break; RG -> RRRG
	//(3, 4) { dest[0] = src[0], dest[1] = src[1], dest[2] = src[2], dest[3] = 255; } break; RGB -> RGB1


	// rgba T(float, byte, short - w/e stb supports) texture
	class Texture : public DiskAsset
	{
	protected:
		uint32 m_width;
		uint32 m_height;
		// actual textures components -> final texture has RGBA, but this value gives insight to how the data is stored
		// however same texture types (e.g heightmaps) may have different actual components (1-grayscale, 4-rgba with r=g=b) based on the image's properties
		// therefore this value isn't very useful 
		uint32 m_components;

		DYNAMIC_RANGE m_dynamicRange;

		void* m_data;

		template<typename T>
		static void CopyValueTypedToTextureBuffer(T* textureBuffer, T* valueBuffer, uint32 size, uint32 components, T extra);

	public:
		Texture(DiskAssetManager* context);
		virtual ~Texture();

		bool Load(const std::string& path, DYNAMIC_RANGE dr, bool flipVertically);
		void Clear() override;

		uint32 GetWidth() const { return m_width; }
		uint32 GetHeight() const { return m_height; }
		uint32 GetComponents() const { return m_components; }
		void* GetData() const { return m_data; }

		DYNAMIC_RANGE GetType() const { return m_dynamicRange; }

		// creates default rgba texture
		static std::unique_ptr<Texture> CreateDefaultTexture(DiskAssetManager* context, void* texelValue, uint32 width,
			uint32 height, uint32 components, DYNAMIC_RANGE dr, const std::string& defaultName = "DefaultTexture");
	
		void ToString(std::ostream& os) const override { os << "asset-type: Texture, name: " << m_label << ", type: " << TexelEnumToString(m_dynamicRange); }

		// use this instead of malloc or new, it uses stbi_malloc and texture will clean up correctly when deleted
		void ReserveTextureDataMemory(uint32 size);
	};
}

#endif // TEXTURE_H
