#pragma once

#include "renderer/Renderer.h"
#include "renderer/NodeObserver.h"
#include "system/EngineEvents.h"

#include <unordered_set>

class ObserverRenderer : public Renderer {
	std::unordered_set<std::unique_ptr<NodeObserverBase>> m_observers;

	// Could be easily modified to allow for user adding functions but currently out of scope
	std::unordered_map<const ReflClass*, std::function<void(Node*)>> m_onTypeAdded;

protected:
	ObserverRenderer()
	{
		m_nodeAddedListener.BindMember(this, &ObserverRenderer::OnNodeAddedToWorld);
		m_nodeRemovedListener.BindMember(this, &ObserverRenderer::OnNodeRemovedFromWorld);
	}

	DECLARE_EVENT_LISTENER(m_nodeAddedListener, Event::OnWorldNodeAdded);
	DECLARE_EVENT_LISTENER(m_nodeRemovedListener, Event::OnWorldNodeRemoved);


	// WIP: Cleanup unused stuff, decide on "Singleton" Observer (maybe even auto-add them?)
	// WIP: Maybe remove custom deleter for each observer instance?
	// TODO: custom dirtyFlagset structure?
	// DOC: document the final version

	template<typename ObserverType>
	[[nodiscard]] ObserverType* CreateObserver_Callback(
		typename ObserverType::NodeType* typedNode, std::function<void(NodeObserverBase*)> onObserveeLost)
	{
		ObserverType* rawPtr = new ObserverType(typedNode);
		m_observers.emplace(std::unique_ptr<NodeObserverBase>(rawPtr));
		rawPtr->onObserveeLost = onObserveeLost;
		return rawPtr;
	}

	template<typename ObserverType>
	ObserverType* CreateObserver_Weak(typename ObserverType::NodeType* typedNode)
	{
		auto lambda = [](NodeObserverBase* obs) -> void {
			dynamic_cast<ObserverType::NodeType*>(obs)->node = nullptr;
			baseNode = nullptr;
		};

		return CreateObserver_Callback<ObserverType>(typedNode, lambda);
	}

	//
	//
	//
	template<typename ObserverType, typename T>
	ObserverType* CreateObserver_AutoContained(
		typename ObserverType::NodeType* typedNode, T& containerToAddAndRemoveFrom)
	{
		auto& cont = containerToAddAndRemoveFrom;

		auto lambda = [&](NodeObserverBase* obs) -> void {
			cont.erase(std::find(cont.begin(), cont.end(), obs));

			m_observers.erase(
				std::find_if(begin(m_observers), end(m_observers), [&](auto& elem) { return elem.get() == obs; }));
		};
		auto rawPtr = CreateObserver_Callback<ObserverType>(typedNode, lambda);
		cont.insert(cont.end(), rawPtr);
		return rawPtr;
	}

	// Attempts to track any available node of this type.
	// When this node gets deleted, it will automatically observe another available node of this type.
	// If no such node is available or the last one from the world gets deleted, it will Nullptr the node member.

	template<typename ObserverType>
	[[nodiscard]] ObserverType* CreateObserver_AnyAvailable()
	{
		auto nodePtr = Engine::GetWorld()->GetAnyAvailableNode<typename ObserverType::NodeType>();

		if (!nodePtr)
			return nullptr;

		auto lambda = [](NodeObserverBase* obs) -> void {
			auto nodePtr = Engine::GetWorld()->GetAnyAvailableNode<typename ObserverType::NodeType>();
			dynamic_cast<ObserverType*>(obs)->node = nodePtr;
			obs->baseNode = nodePtr;
		};

		return CreateObserver_Callback<ObserverType>(nodePtr, lambda);
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

		// First fill the container
		auto world = Engine::GetWorld();
		for (auto node : world->GetNodeIterator<NodeType>()) {
			CreateObserver_AutoContained<ObserverType>(node, containerToAddAndRemoveFrom);
		}

		// Our on add lambda
		auto adderLambda = [&](Node* nodeToAdd) {
			CLOG_ABORT(nodeToAdd->GetClass() != NodeType::StaticClass(),
				"Incorrect type, Static cast will fail. Observer Renderer Internal error");

			CreateObserver_AutoContained<ObserverType>(static_cast<NodeType*>(nodeToAdd), containerToAddAndRemoveFrom);
		};

		// Register our add function
		m_onTypeAdded.insert({ &NodeType::StaticClass(), std::move(adderLambda) });
	}

	void RemoveObserver(NodeObserverBase* ptr);

private:
	void OnNodeRemovedFromWorld(Node* node);

protected:
	virtual void OnNodeAddedToWorld(Node* node);

public:
	virtual void Update();

	virtual void ActiveCameraResize(){};
};
