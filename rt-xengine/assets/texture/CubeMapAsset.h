#pragma once

#include "assets/texture/TextureAsset.h"

class CubeMapAsset : public Asset
{
	TextureAsset* m_faces[CMF_COUNT];

	uint32 m_width;
	uint32 m_height;

	bool m_hdr;

public:
	CubeMapAsset(const fs::path& path)
		: Asset(path),
	      m_faces{nullptr},
		  m_width(0),
		  m_height(0),
		  m_hdr(false) {}

	uint32 GetWidth() const { return m_width; }
	uint32 GetHeight() const { return m_height; }
	bool IsHdr() const { return m_hdr; }

	TextureAsset* GetFace(CubeMapFace faceIndex) const { return m_faces[faceIndex]; }
	
protected:
	bool Load() override;
	void Unload() override;
};
