#pragma once
#include "renderer/NodeObserver.h"
#include "renderer/renderers/opengl/GLRendererBase.h"
#include "asset/AssetPod.h"
#include "asset/PodHandle.h"

namespace OpenGL
{
	struct GLAssetBase : public RendererObject<GLRendererBase>
	{
		GLAssetBase(BasePodHandle genericHandle)
			: genericPodHandle(genericHandle) {}

		BasePodHandle genericPodHandle;

		virtual ~GLAssetBase() = default;
	};

	template<typename PodTypeT>
	struct GLAsset : public GLAssetBase
	{
	public:
		using PodType = PodTypeT;

	protected:
		GLAsset(PodHandle<PodType> handle)
			: GLAssetBase(handle)
			, podHandle(handle) {}

		PodHandle<PodType> podHandle;
		
		virtual bool Load() = 0;
		virtual ~GLAsset() = default;

		friend GenericGpuAssetManager<GLAssetBase>;
	private:
		void FriendLoad() {	Load(); }
	public:
		// DOC: 
		const PodType* LockData()
		{
			return podHandle.Lock();
		}
	};

}
