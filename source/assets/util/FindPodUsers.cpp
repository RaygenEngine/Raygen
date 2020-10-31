#include "FindPodUsers.h"

#include "reflection/ReflectionTools.h"

struct IsUserOfPodVisitor {
	size_t uid;
	bool result{ false };

	IsUserOfPodVisitor(PodEntry* pod)
		: uid(pod->uid)
	{
	}

	// Returning false from here skips the the property.
	// We return false everywhere when we have already found that this is a user.
	bool PreProperty(const Property& p)
	{
		if (result == true) {
			return false;
		}
		return true;
	}

	template<typename T>
	void operator()(T& ref, const Property& p)
	{
	}

	void operator()(BasePodHandle& ref, const Property& p)
	{
		if (ref.uid == uid) {
			result = true;
		}
	}

	template<typename T>
	void operator()(std::vector<PodHandle<T>>& ref, const Property& p)
	{
		for (auto& handle : ref) {
			if (handle.uid == uid) {
				result = true;
				return;
			}
		}
	}
};


std::vector<PodEntry*> FindAssetUsersOfPod(PodEntry* pod)
{
	auto& pods = AssetRegistry::Z_GetPods();

	std::vector<PodEntry*> results;

	for (auto& entry : pods) {
		[[unlikely]] if (entry->transient) { continue; }

		IsUserOfPodVisitor v(pod);

		refltools::CallVisitorOnEveryProperty(entry->ptr.get(), v);

		if (v.result) {
			results.push_back(entry.get());
		}
	}

	return results;
}
// TODO: ECS
//
// std::vector<Node*> FindComponentUsersOfPod(PodEntry* pod, World& world)
//{
//	std::vector<Node*> users;
//
//
//	// for (Node* node : world->GetNodes()) {
//	//	IsUserOfPodVisitor v(pod);
//
//	//	refltools::CallVisitorOnEveryProperty(node, v);
//
//	//	if (v.result) {
//	//		users.push_back(node);
//	//	}
//	//}
//
//	return users;
//}
