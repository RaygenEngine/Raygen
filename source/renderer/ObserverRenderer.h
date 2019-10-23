#pragma once

#include "renderer/Renderer.h"
#include "renderer/NodeObserver.h"
#include "system/EngineEvents.h"

#include <unordered_set>

class ObserverRenderer : public Renderer {
	std::unordered_set<std::unique_ptr<NodeObserverBase>> m_observers;

	// Could be easily modified to allow for user adding functions but currently out of scope
	std::unordered_map<const ReflClass*, std::function<void(Node*)>> m_onTypeAdded;
	std::unordered_map<const ReflClass*, std::function<void(Node*)>> m_onTypeRemoved;

protected:
	ObserverRenderer()
	{
		m_nodeAddedListener.BindMember(this, &ObserverRenderer::OnNodeAddedToWorld);
		m_nodeRemovedListener.BindMember(this, &ObserverRenderer::OnNodeRemovedFromWorld);
	}

	DECLARE_EVENT_LISTENER(m_nodeAddedListener, Event::OnWorldNodeAdded);
	DECLARE_EVENT_LISTENER(m_nodeRemovedListener, Event::OnWorldNodeRemoved);


	// WIP: Cleanup unused stuff, decide on "Singleton" Observer (maybe even auto-add them?)
	// TODO: custom dirtyFlagset structure?
	// DOC: document the final version

	template<typename NodeType>
	[[nodiscard]] void RegisterObserver_PointerForTracking(NodeType*& ptrToAutoUpdate)
	{
		auto world = Engine::GetWorld();

		auto findNext = [&, ptrToAutoUpdate]() {
			auto world = Engine::GetWorld();
			ptrToAutoUpdate = world->GetAnyAvailableNode<NodeType>();

			if (!ptrToAutoUpdate) {
				m_onTypeAdded.insert({ &NodeType::StaticClass(), findNext });
				return;
			}
			m_onTypeAdded.erase(&NodeType::StaticClass());
		};


		return nullptr;
	}

	// Attempts to track any available node of this type.
	// When this node gets deleted, it will automatically observe another available node of this type.
	// If no such node is available or the last one from the world gets deleted, it will Nullptr the node member.
	template<typename ObserverType>
	[[nodiscard]] ObserverType* CreateObserver_AnyAvailable()
	{

		return nullptr;
	}


	// Tracks ALL nodes of this type AND subtypes by automatically adding and removing them from the observer container.
	// You can handle additional creation/update/deletion stuff through the Observer's:
	// Constructor/DirtyNodeUpdate/Destructor respectively
	// NOTE: observer type is deduced from container::value_type
	template<typename T>
	void RegisterObserverContainer_AutoLifetimes(T& containerToAddAndRemoveFrom)
	{
		using ObserverType = std::remove_pointer_t<typename T::value_type>;
		using NodeType = typename ObserverType::NodeType;

		// Fill the container with the current nodes of this type
		auto world = Engine::GetWorld();
		for (auto node : world->GetNodeIterator<NodeType>()) {
			CreateAddObserverToContainer<ObserverType>(node, containerToAddAndRemoveFrom);
		}

		// On add lambda
		auto adderLambda = [&](Node* nodeToAdd) {
			CreateAddObserverToContainer<ObserverType>(NodeCast<NodeType>(nodeToAdd), containerToAddAndRemoveFrom);
		};

		// Register our add function
		m_onTypeAdded.insert({ &NodeType::StaticClass(), std::move(adderLambda) });
	}

	void RemoveObserver(NodeObserverBase* ptr);

protected:
	// Probably worthless to overload this under normal circumstances, you should prefer to use the automatic lifetimes
	// of the observer renderer
	virtual void OnNodeRemovedFromWorld(Node* node);
	virtual void OnNodeAddedToWorld(Node* node);
	virtual void ActiveCameraResize(){};

public:
	virtual void Update();


private:
	// Internal utility function
	template<typename ObserverType, typename T>
	ObserverType* CreateAddObserverToContainer(
		typename ObserverType::NodeType* typedNode, T& containerToAddAndRemoveFrom)
	{
		auto& cont = containerToAddAndRemoveFrom;

		auto lambda = [&](NodeObserverBase* obs) -> void {
			cont.erase(std::find(cont.begin(), cont.end(), obs));

			m_observers.erase(
				std::find_if(begin(m_observers), end(m_observers), [&](auto& elem) { return elem.get() == obs; }));
		};

		ObserverType* rawPtr = new ObserverType(typedNode);
		m_observers.emplace(std::unique_ptr<NodeObserverBase>(rawPtr));
		rawPtr->onObserveeLost = lambda;

		containerToAddAndRemoveFrom.insert(cont.end(), rawPtr);
		return rawPtr;
	}
};
