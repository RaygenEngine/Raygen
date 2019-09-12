#pragma once

#include "asset/Asset.h"
#include "asset/pods/SamplerPod.h"

class CubemapAsset : public Asset
{
	SamplerPod m_sides[CMF_COUNT];

	int32 m_width;
	int32 m_height;

	bool m_hdr;

public:
	CubemapAsset(const fs::path& path)
		: Asset(path),
		  m_width(0),
		  m_height(0),
		  m_hdr(false) 
	{
		m_reflector.AddProperty("front", m_sides[CMF_FRONT].image);
		m_reflector.AddProperty("back", m_sides[CMF_BACK].image);
		m_reflector.AddProperty("right", m_sides[CMF_RIGHT].image);
		m_reflector.AddProperty("left", m_sides[CMF_LEFT].image);
		m_reflector.AddProperty("top", m_sides[CMF_UP].image);
		m_reflector.AddProperty("bottom", m_sides[CMF_DOWN].image);
	}

	[[nodiscard]] int32 GetWidth() const { return m_width; }
	[[nodiscard]] int32 GetHeight() const { return m_height; }
	[[nodiscard]] bool IsHdr() const { return m_hdr; }

	[[nodiscard]] SamplerPod& GetFace(CubeMapFace faceIndex) { return m_sides[faceIndex]; }
	[[nodiscard]] SamplerPod& GetFace(int32 faceIndex) { return m_sides[faceIndex]; }
	
	static bool LoadAll(CubemapAsset* asset);
protected:
	bool Load() override;
	void Unload() override;
};
