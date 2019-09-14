#pragma once

#include "asset/Asset.h"
#include "asset/pods/CubemapPod.h"

class CubemapAsset : public PodedAsset<CubemapPod>
{
public:
	CubemapAsset(const fs::path& path)
		: PodedAsset(path)
	{
		//m_reflector.AddProperty("front", m_cubemapData.sides[CMF_FRONT].image);
		//m_reflector.AddProperty("back", m_cubemapData.sides[CMF_BACK].image);
		//m_reflector.AddProperty("right", m_cubemapData.sides[CMF_RIGHT].image);
		//m_reflector.AddProperty("left", m_cubemapData.sides[CMF_LEFT].image);
		//m_reflector.AddProperty("top", m_cubemapData.sides[CMF_UP].image);
		//m_reflector.AddProperty("bottom", m_cubemapData.sides[CMF_DOWN].image);
	}
	
protected:
	bool Load() override;
};
