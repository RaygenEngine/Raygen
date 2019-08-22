#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "assets/DiskAsset.h"

#include "Texture.h"

namespace Assets
{
	// gl cubemap based ordering
	enum CUBE_MAP_FACE
	{
		CMF_RIGHT = 0,
		CMF_LEFT,
		CMF_UP,
		CMF_DOWN,
		CMF_FRONT,
		CMF_BACK,
		CMF_COUNT
	};

	// rgba T(float, byte, short - w/e stb supports) texture
	class CubeMap : public DiskAsset
	{
		std::shared_ptr<Texture> m_faces[CMF_COUNT];

		// must be same for all faces
		uint32 m_width;
		uint32 m_height;

		DYNAMIC_RANGE m_dynamicRange;

	public:
		CubeMap(DiskAssetManager* context);

		bool Load(const std::string& path, DYNAMIC_RANGE dr, bool flipVertically);
		void Clear() override;

		uint32 GetWidth() const { return m_width; }
		uint32 GetHeight() const { return m_height; }

		Texture* GetFace(CUBE_MAP_FACE faceIndex) const { return m_faces[faceIndex].get(); }

		DYNAMIC_RANGE GetType() const { return m_dynamicRange; }

		void ToString(std::ostream& os) const override { os << "asset-type: CubeMap, name: " << m_label << ", type: " << TexelEnumToString(m_dynamicRange); }
	};
}

#endif // CUBEMAP_H
