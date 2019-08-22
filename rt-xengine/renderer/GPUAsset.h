#ifndef GPUASSET_H
#define GPUASSET_H

#include "Renderer.h"

namespace Renderer
{
	
	class GPUAsset : public Assets::Asset
	{
	protected:
		// will see if this helps
		Renderer* m_renderer;
		std::string m_associatedDescription;


	public:
		explicit GPUAsset(Renderer* renderer);
		virtual ~GPUAsset() = default;

		void SetIdentificationFromAssociatedDiskAssetIdentification(const std::string& associatedDescription);

		std::string GetAssociatedDescription() const { return m_associatedDescription; }
	};

	

}

#endif // GPUASSET_H
