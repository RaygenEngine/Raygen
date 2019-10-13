#pragma once

#include "asset/AssetPod.h"
#include "asset/PodHandle.h"
#include "renderer/renderers/opengl/GLRendererBase.h"

namespace ogl {
struct GLAssetBase
	: Object
	, RendererObject<GLRendererBase> {

	GLAssetBase(BasePodHandle genericHandle)
		: genericPodHandle(genericHandle)
	{
	}

	BasePodHandle genericPodHandle;
};

template<typename PodTypeT>
struct GLAsset : GLAssetBase {

	using PodType = PodTypeT;

protected:
	GLAsset(PodHandle<PodType> handle)
		: GLAssetBase(handle)
		, podHandle(handle)
	{
	}

	PodHandle<PodType> podHandle;

	virtual void Load() = 0;

private:
	friend GenericGpuAssetManager<GLAssetBase>;
	void FriendLoad() { Load(); }

public:
	// DOC:
	const PodType* LockData() { return podHandle.Lock(); }
};

} // namespace ogl
