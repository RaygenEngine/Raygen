#include "pch.h"
#include "GPUAsset.h"

namespace Renderer
{
	GPUAsset::GPUAsset(Renderer* renderer)
		: Asset(renderer),
		  m_renderer(renderer)
	{
	}

	void GPUAsset::SetIdentificationFromAssociatedDiskAssetIdentification(const std::string& associatedDescription)
	{
		m_associatedDescription = associatedDescription;
	}
}
