#pragma once

#include "assets/texture/TextureAsset.h"

// TODO: rename to CubeMapContainer
class CubeMapAsset : public Asset
{
	TextureAsset* m_faces[CMF_COUNT];

	int32 m_width;
	int32 m_height;

	bool m_hdr;

public:
	CubeMapAsset(const fs::path& path)
		: Asset(path),
		  m_faces{nullptr},
		  m_width(0),
		  m_height(0),
		  m_hdr(false) {}

	[[nodiscard]] int32 GetWidth() const { return m_width; }
	[[nodiscard]] int32 GetHeight() const { return m_height; }
	[[nodiscard]] bool IsHdr() const { return m_hdr; }

	[[nodiscard]] TextureAsset* GetFace(CubeMapFace faceIndex) const { return m_faces[faceIndex]; }
	
protected:
	bool Load() override;
	void Unload() override;
};
