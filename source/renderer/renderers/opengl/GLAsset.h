#pragma once
#include "renderer/NodeObserver.h"
#include "renderer/renderers/opengl/GLRendererBase.h"
#include "asset/AssetPod.h"
#include "asset/PodHandle.h"

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
struct GLAsset : public GLAssetBase {
public:
	using PodType = PodTypeT;

protected:
	GLAsset(PodHandle<PodType> handle)
		: GLAssetBase(handle)
		, podHandle(handle)
	{
	}

	PodHandle<PodType> podHandle;

	virtual bool Load() = 0;

private:
	friend GenericGpuAssetManager<GLAssetBase>;
	void FriendLoad() { Load(); }

public:
	// DOC:
	const PodType* LockData() { return podHandle.Lock(); }
};

} // namespace ogl
