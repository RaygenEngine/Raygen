#include "Scene.h"
#pragma once

template<CSceneElem T>
inline T* Scene::GetElement(size_t uid)
{
	return Get<T>().Get(uid);
}

template<CSceneElem T>
inline void Scene::SetCtx()
{
	ctxHash = mti::GetHash<T>();
	ctxCollection = collections[ctxHash].get();
}

template<CSceneElem T>
inline void Scene::EnqueueCmdInCtx(size_t uid, std::function<void(T&)>&& command)
{
	CLOG_ABORT(ctxHash != mti::GetHash<T>(),
		"EnqueueCmdInCtx was called with incorrect context. Use SetCtx before this call or EnqueueCmd if you only "
		"want a single cmd (slower per call).");

	currentCmdBuffer->emplace_back(
		[&, cmd = std::move(command), collection = static_cast<SceneCollection<T>*>(ctxCollection), uid]() {
			T& sceneElement = *collection->Get(uid);
			cmd(sceneElement);
			auto& dirtyVec = sceneElement.isDirty;
			std::fill(dirtyVec.begin(), dirtyVec.end(), true);
		});
}

template<CSceneElem T>
inline void Scene::EnqueueCmd(size_t uid, std::function<void(T&)>&& command)
{
	currentCmdBuffer->emplace_back([&, cmd = std::move(command), uid]() {
		T& sceneElement = *Get<T>().Get(uid);
		cmd(sceneElement);
		auto& dirtyVec = sceneElement.isDirty;
		std::fill(dirtyVec.begin(), dirtyVec.end(), true);
	});
}

template<CSceneElem T>
void Scene::EnqueueCreateDestoryCmds(std::vector<size_t>&& destructions, std::vector<size_t*>&& outCreations)
{
	if (destructions.empty() && outCreations.empty()) {
		return;
	}

	// std::lock_guard<std::mutex> guard(cmdAddPendingElementsMutex);
	SceneCollection<T>& type = Get<T>();
	for (auto& uid : destructions) {
		type.RemoveUid(uid);
	}

	std::vector<size_t> constructions;

	for (auto& scUid : outCreations) {
		size_t uid = type.GetNextUid();
		*scUid = uid;
		constructions.emplace_back(uid);
	}

	currentCmdBuffer->emplace_back([&, destr = std::move(destructions), constr = std::move(constructions)]() {
		// TODO: deferred deleting of scene objects
		vl::Device->waitIdle();
		for (auto uid : destr) {
			delete type.Get(uid);
			type.Set(uid, nullptr);

			// Condensing
			// Swap & Pop using condensedLocation to locate elements
			type.SwapPopFromCondensed(uid);
		}

		for (auto uid : constr) {
			type.Set(uid, new T());
			// Condensing
			type.condensed.emplace_back(type.Get(uid));
			type.condensedToUid.emplace_back(uid);
			type.condensedLocation[uid] = type.condensed.size() - 1;
		}
	});
}

template<CSceneElem T>
SceneCollection<T>& Scene::Get()
{
	return *static_cast<SceneCollection<T>*>(collections.at(mti::GetHash<T>()).get());
}
