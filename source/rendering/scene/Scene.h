#pragma once

#include "rendering/scene/SceneStructs.h"
#include <functional>


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

inline struct S_Scene {
	S_Scene() { EnqueueEndFrame(); } // WIP:

	SceneVector<SceneGeometry> geometries;
	SceneVector<SceneCamera> cameras;
	SceneVector<SceneSpotlight> spotlights;

	std::vector<UniquePtr<std::vector<std::function<void()>>>> commands;

	std::vector<std::function<void()>>* currentCommandBuffer;

	std::mutex cmdBuffersVectorMutex;
	std::mutex cmdAddPendingElementsMutex;

	size_t activeCamera{ 0 };


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
		LOG_ABORT("Incorrect type");
	}

	template<CONC(CSceneElem) T>
	void EnqueueCmd(size_t uid, std::function<void(T&)>&& command)
	{
		currentCommandBuffer->emplace_back([uid, cmd = std::move(command)]() {
			//
			cmd(*Scene->GetElement<T>(uid));
		});
	}

	template<CONC(CSceneElem) T>
	size_t EnqueueCreateCmd()
	{
		// std::lock_guard<std::mutex> guard(cmdAddPendingElementsMutex);

		size_t uid{};
		if constexpr (std::is_same_v<SceneGeometry, T>) {
			uid = geometries.elements.size() + geometries.pendingElements++;
			currentCommandBuffer->emplace_back([uid]() { Scene->geometries.elements[uid] = new SceneGeometry(); });
		}
		else if constexpr (std::is_same_v<SceneCamera, T>) {
			uid = cameras.elements.size() + cameras.pendingElements++;
			currentCommandBuffer->emplace_back([uid]() { Scene->cameras.elements[uid] = new SceneCamera(); });
		}
		else if constexpr (std::is_same_v<SceneSpotlight, T>) {
			uid = spotlights.elements.size() + spotlights.pendingElements++;
			currentCommandBuffer->emplace_back([uid]() { Scene->spotlights.elements[uid] = new SceneSpotlight(); });
		}
		return uid;
	}


	template<CONC(CSceneElem) T>
	void EnqueueDestroyCmd(size_t uid)
	{
		if constexpr (std::is_same_v<SceneGeometry, T>) {
			currentCommandBuffer->emplace_back([uid]() {
				auto elem = static_cast<T*>(Scene->geometries.elements[uid]);
				Scene->geometries.elements[uid] = nullptr;
				delete elem;
			});
		}
		else if constexpr (std::is_same_v<SceneCamera, T>) {
			currentCommandBuffer->emplace_back([uid]() {
				auto elem = static_cast<T*>(Scene->cameras.elements[uid]);
				Scene->cameras.elements[uid] = nullptr;
				delete elem;
			});
		}
		else if constexpr (std::is_same_v<SceneSpotlight, T>) {
			currentCommandBuffer->emplace_back([uid]() {
				auto elem = static_cast<T*>(Scene->spotlights.elements[uid]);
				Scene->spotlights.elements[uid] = nullptr;
				delete elem;
			});
		}
	}


	void EnqueueEndFrame()
	{
		// std::lock_guard<std::mutex> guard(cmdBuffersVectorMutex);
		currentCommandBuffer = commands.emplace_back(std::make_unique<std::vector<std::function<void()>>>()).get();
	}

	void EnqueueActiveCameraCmd(size_t uid)
	{
		currentCommandBuffer->emplace_back([uid]() { Scene->activeCamera = uid; });
	}

private:
	void ExecuteCreations()
	{
		// std::lock_guard<std::mutex> guard(cmdAddPendingElementsMutex);
		geometries.AppendPendingElements();
		cameras.AppendPendingElements();
		spotlights.AppendPendingElements();
	}

public:
	void ConsumeCmdQueue()
	{
		ExecuteCreations();

		size_t begin = 0;
		size_t end = commands.size() - 1;

		if (end <= 0) {
			return;
		}

		for (size_t i = 0; i < end; ++i) {
			for (auto& cmd : *commands[i]) {
				cmd();
			}
		}


		// std::lock_guard<std::mutex> guard(cmdBuffersVectorMutex);
		commands.erase(commands.begin(), commands.begin() + end);
	}

} * Scene;
