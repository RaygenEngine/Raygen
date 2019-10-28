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
		m_activeCameraChangedListener.BindMember(this, &ObserverRenderer::OnActiveCameraChanged);
	}

	DECLARE_EVENT_LISTENER(m_nodeAddedListener, Event::OnWorldNodeAdded);
	DECLARE_EVENT_LISTENER(m_nodeRemovedListener, Event::OnWorldNodeRemoved);
	DECLARE_EVENT_LISTENER(m_activeCameraChangedListener, Event::OnWorldActiveCameraChanged);


	[[nodiscard]] const std::unordered_set<std::unique_ptr<NodeObserverBase>>& GetObservers() const
	{
		return m_observers;
	}

	// WIP: Cleanup unused stuff, decide on "Singleton" Observer (maybe even auto-add them?)
	// TODO: custom dirtyFlagset structure?
	// PERF: remove_swap for vectors and fast iterations
	// DOC: document the final version


	// Attempts to track any available node of this type.
	// When this node gets deleted, it will automatically observe another available node of this type.
	// If no such node is available or the last one from the world gets deleted, it will Nullptr the node member.
	// Note: observer constructor is called with nullptr node if no such node type is found.
	template<typename ObserverType>
	[[nodiscard]] ObserverType* CreateTrackerObserver_AnyAvailableWithCallback(
		std::function<void(ObserverType*)>&& callbackWhenNodeChanged);


	// Tracks ALL nodes of this type AND subtypes by automatically adding and removing them from the observer container.
	// You can handle additional creation/update/deletion stuff through the Observer's:
	// Constructor/DirtyNodeUpdate/Destructor respectively
	// NOTE: observer type is deduced from container::value_type
	// DOC: Limitation, cannot track the same node with 2 observers, each observer must have a one to one relation to a
	// node
	template<typename T>
	void RegisterObserverContainer_AutoLifetimes(T& containerToAddAndRemoveFrom);

	// For simplification purposes
	CameraNode* m_activeCamera{ nullptr };

protected:
	// Probably worthless to overload this under normal circumstances, you should prefer to use the automatic lifetimes
	// of the observer renderer
	virtual void OnNodeRemovedFromWorld(Node* node);
	virtual void OnNodeAddedToWorld(Node* node);
	virtual void OnActiveCameraChanged(CameraNode* node);
	virtual void ActiveCameraResize(){};

public:
	virtual void Update();
	virtual void Render() = 0;
	void DrawFrame() final override;


	// Do not use this with NodeTypes that are already handled for automatic lifetimes. This deregisters the other
	// functions for this type
	template<typename NodeType>
	void RegisterAdderRemoverForType(std::function<void(Node*)>&& adderFunc, std::function<void(Node*)>&& removerFunc)
	{
		m_onTypeAdded.insert({ &NodeType::StaticClass(), adderFunc });
		m_onTypeRemoved.insert({ &NodeType::StaticClass(), removerFunc });
	}
};


template<typename ObserverType>
ObserverType* ObserverRenderer::CreateTrackerObserver_AnyAvailableWithCallback(
	std::function<void(ObserverType*)>&& callbackWhenNodeChanged)
{
	using NodeType = typename ObserverType::NodeType;

	NodeType* node = Engine::GetWorld()->GetAnyAvailableNode<NodeType>();
	ObserverType* observer = new ObserverType(node);
	m_observers.emplace(std::unique_ptr<NodeObserverBase>(observer));

	auto onAdd = [callback = callbackWhenNodeChanged, observer](Node* node) {
		if (!observer->baseNode) {
			observer->baseNode = node;
			observer->node = NodeCast<NodeType>(node);
			callback(observer);
		}
	};

	auto onRemove = [&, callback{ std::move(callbackWhenNodeChanged) }, observer](Node* node) {
		if (node == observer->node) {
			NodeType* newNode = Engine::GetWorld()->GetAnyAvailableNode<NodeType>();
			observer->baseNode = newNode;
			observer->node = newNode ? NodeCast<NodeType>(newNode) : nullptr;
			callback(observer);
		}
	};

	RegisterAdderRemoverForType<NodeType>(onAdd, onRemove);

	return observer;
}


template<typename T>
void ObserverRenderer::RegisterObserverContainer_AutoLifetimes(T& containerToAddAndRemoveFrom)
{
	using ObserverType = std::remove_pointer_t<typename T::value_type>;
	using NodeType = typename ObserverType::NodeType;

	// On add lambda
	auto adderLambda = [&](Node* nodeToAdd) {
		ObserverType* rawPtr = new ObserverType(NodeCast<NodeType>(nodeToAdd));
		m_observers.emplace(std::unique_ptr<NodeObserverBase>(rawPtr));
		containerToAddAndRemoveFrom.insert(containerToAddAndRemoveFrom.end(), rawPtr);
	};

	// Fill the container with the current nodes of this type
	auto world = Engine::GetWorld();
	for (auto node : world->GetNodeIterator<NodeType>()) {
		adderLambda(node);
	}

	auto removerLambda = [&](Node* nodeToRemove) {
		auto& cont = containerToAddAndRemoveFrom;
		cont.erase(std::remove_if(cont.begin(), cont.end(),
			[nodeToRemove](ObserverType* obsPtr) { return obsPtr->baseNode == nodeToRemove; }));

		m_observers.erase(std::find_if(m_observers.begin(), m_observers.end(),
			[nodeToRemove](const std::unique_ptr<NodeObserverBase>& ptr) { return ptr->baseNode == nodeToRemove; }));
	};

	RegisterAdderRemoverForType<NodeType>(adderLambda, removerLambda);
}
