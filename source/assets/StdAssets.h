#pragma once


struct StdAssets {

	template<CAssetPod T>
	struct StdAsset {
		PodHandle<T> handle;

		bool initialized{ false };

		StdAsset() {}

		void operator=(const PodHandle<T>&) = delete;

		struct ConstHandleAssigner {
			size_t uid;
		};

		void operator=(ConstHandleAssigner assigner)
		{
			CLOG_ABORT(initialized, "Reinitializing already initialized ConstHandle");
			handle.uid = assigner.uid;
			initialized = true;
		}

		PodHandle<T> operator()()
		{
			CLOG_ERROR(!initialized, "Accessed uninitialized StdAsset handle. Default will be returned instead.");
			return handle;
		}

		void operator=(const StdAsset<T>&) = delete;
		StdAsset(const StdAsset<T>&) = delete;
		void operator=(StdAsset<T>&&) = delete;
		StdAsset(StdAsset<T>&&) = delete;
	};


	inline static StdAsset<Image> NormalImage;
	inline static StdAsset<Image> BrdfLut;
	inline static StdAsset<MaterialArchetype> GltfArchetype;
	inline static StdAsset<Mesh> QuadMesh;
	inline static StdAsset<Mesh> UnitCube;


	static void LoadAssets();
};
