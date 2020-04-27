#pragma once
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/scene/SceneReflectionProbe.h"
#include "rendering/scene/SceneSpotlight.h"

#include <functional>

template<typename T>
concept CSceneElem
	= std::is_same_v<SceneGeometry,
		  T> || std::is_same_v<SceneCamera, T> || std::is_same_v<SceneSpotlight, T> || std::is_same_v<SceneReflectionProbe, T>;

template<CONC(CSceneElem) T>
struct SceneVector {
	std::vector<T*> elements;
	size_t pendingElements{ 0 };


	void AppendPendingElements()
	{
		elements.resize(elements.size() + pendingElements);
		pendingElements = 0;
	}
};

inline struct Scene_ {
	SceneVector<SceneGeometry> geometries;
	SceneVector<SceneCamera> cameras;
	SceneVector<SceneSpotlight> spotlights;
	SceneVector<SceneReflectionProbe> reflProbs;

	std::vector<UniquePtr<std::vector<std::function<void()>>>> cmds;
	std::vector<std::function<void()>>* currentCmdBuffer;

	std::mutex cmdBuffersVectorMutex;
	std::mutex cmdAddPendingElementsMutex;

	size_t activeCamera{ 0 };
	size_t size{ 0 };

	template<CONC(CSceneElem) T>
	T* GetElement(size_t uid)
	{
		if constexpr (std::is_same_v<SceneGeometry, T>) {
			return geometries.elements.at(uid);
		}
		else if constexpr (std::is_same_v<SceneCamera, T>) {
			return cameras.elements.at(uid);
		}
		else if constexpr (std::is_same_v<SceneSpotlight, T>) {
			return spotlights.elements.at(uid);
		}
		else if constexpr (std::is_same_v<SceneReflectionProbe, T>) {
			return reflProbs.elements.at(uid);
		}
		LOG_ABORT("Incorrect type");
	}

	template<CONC(CSceneElem) T>
	void EnqueueCmd(size_t uid, std::function<void(T&)>&& command)
	{
		currentCmdBuffer->emplace_back([uid, cmd = std::move(command)]() {
			//
			cmd(*Scene->GetElement<T>(uid));
			auto& dirtyVec = Scene->GetElement<T>(uid)->isDirty;
			std::fill(dirtyVec.begin(), dirtyVec.end(), true);
		});
	}

	template<CONC(CSceneElem) T>
	size_t EnqueueCreateCmd()
	{
		// std::lock_guard<std::mutex> guard(cmdAddPendingElementsMutex);

		size_t uid{};
		if constexpr (std::is_same_v<SceneGeometry, T>) {
			uid = geometries.elements.size() + geometries.pendingElements++;
			currentCmdBuffer->emplace_back([uid]() { Scene->geometries.elements[uid] = new SceneGeometry(); });
		}
		else if constexpr (std::is_same_v<SceneCamera, T>) {
			uid = cameras.elements.size() + cameras.pendingElements++;
			currentCmdBuffer->emplace_back([=]() { Scene->cameras.elements[uid] = new SceneCamera(); });
		}
		else if constexpr (std::is_same_v<SceneSpotlight, T>) {
			uid = spotlights.elements.size() + spotlights.pendingElements++;
			currentCmdBuffer->emplace_back([=]() { Scene->spotlights.elements[uid] = new SceneSpotlight(); });
		}
		else if constexpr (std::is_same_v<SceneReflectionProbe, T>) {
			uid = reflProbs.elements.size() + reflProbs.pendingElements++;
			currentCmdBuffer->emplace_back([=]() { Scene->reflProbs.elements[uid] = new SceneReflectionProbe(); });
		}

		return uid;
	}


	template<CONC(CSceneElem) T>
	void EnqueueDestroyCmd(size_t uid)
	{
		if constexpr (std::is_same_v<SceneGeometry, T>) {
			currentCmdBuffer->emplace_back([uid]() {
				auto elem = static_cast<T*>(Scene->geometries.elements[uid]);
				Scene->geometries.elements[uid] = nullptr;
				// TODO: deferred deleting of scene objects
				vl::Device->waitIdle();
				delete elem;
			});
		}
		else if constexpr (std::is_same_v<SceneCamera, T>) {
			currentCmdBuffer->emplace_back([uid]() {
				auto elem = static_cast<T*>(Scene->cameras.elements[uid]);
				Scene->cameras.elements[uid] = nullptr;
				vl::Device->waitIdle();
				delete elem;
			});
		}
		else if constexpr (std::is_same_v<SceneSpotlight, T>) {
			currentCmdBuffer->emplace_back([uid]() {
				auto elem = static_cast<T*>(Scene->spotlights.elements[uid]);
				Scene->spotlights.elements[uid] = nullptr;
				vl::Device->waitIdle();
				delete elem;
			});
		}
		else if constexpr (std::is_same_v<SceneReflectionProbe, T>) {
			currentCmdBuffer->emplace_back([uid]() {
				auto elem = static_cast<T*>(Scene->reflProbs.elements[uid]);
				Scene->reflProbs.elements[uid] = nullptr;
				vl::Device->waitIdle();
				delete elem;
			});
		}
	}

	void EnqueueEndFrame()
	{
		// std::lock_guard<std::mutex> guard(cmdBuffersVectorMutex);
		currentCmdBuffer = cmds.emplace_back(std::make_unique<std::vector<std::function<void()>>>()).get();
	}

	void EnqueueActiveCameraCmd(size_t uid)
	{
		currentCmdBuffer->emplace_back([uid]() { Scene->activeCamera = uid; });
	}

private:
	void ExecuteCreations()
	{
		// std::lock_guard<std::mutex> guard(cmdAddPendingElementsMutex);
		geometries.AppendPendingElements();
		cameras.AppendPendingElements();
		spotlights.AppendPendingElements();
		reflProbs.AppendPendingElements();
	}

public:
	void ConsumeCmdQueue()
	{
		ExecuteCreations();

		size_t begin = 0;
		size_t end = cmds.size() - 1;

		if (end <= 0) {
			return;
		}

		for (size_t i = 0; i < end; ++i) {
			for (auto& cmd : *cmds[i]) {
				cmd();
			}
		}


		// std::lock_guard<std::mutex> guard(cmdBuffersVectorMutex);
		cmds.erase(cmds.begin(), cmds.begin() + end);
	}

	void DrainQueueForDestruction();

	SceneCamera* GetActiveCamera()
	{
		if (cameras.elements.size() > activeCamera) { // PERF:
			auto cam = cameras.elements[activeCamera];
			if (cam) {
				return cam;
			}
		}
		return nullptr;
	}

	// CHECK: runs 2 frames behind
	Scene_(size_t size);

	// TODO: remove
	vk::DescriptorSet GetActiveCameraDescSet();

	void UploadDirty();


	~Scene_() { DrainQueueForDestruction(); }

} * Scene{};
