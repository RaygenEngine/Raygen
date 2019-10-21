#pragma once

#include "renderer/Renderer.h"
#include "renderer/NodeObserver.h"
#include "system/EngineEvents.h"

#include <unordered_set>

class ObserverRenderer : public Renderer {
	std::unordered_set<std::unique_ptr<NodeObserverBase>> m_observers;

protected:
	ObserverRenderer()
	{
		m_nodeAddedListener.BindMember(this, &ObserverRenderer::OnNodeAddedToWorld);
		m_nodeRemovedListener.BindMember(this, &ObserverRenderer::OnNodeRemovedFromWorld);
	}

	DECLARE_EVENT_LISTENER(m_nodeAddedListener, Event::OnWorldNodeAdded);
	DECLARE_EVENT_LISTENER(m_nodeRemovedListener, Event::OnWorldNodeRemoved);

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

	// No use case for this currently
	// template <typename ObserverType>
	// void CreateObserver_SelfDestruct(typename ObserverType::NodeType* typedNode)
	//{
	//	auto lambda = [m_observers](NodeObserverBase* obs) -> void {
	//		&m_observers.erase(obs);
	//	};
	//	auto rawPtr = CreateObserver_Callback(nodePtr, lambda);
	//	return rawPtr;
	//}

	void RemoveObserver(NodeObserverBase* ptr);

private:
	void OnNodeRemovedFromWorld(Node* node);

protected:
	virtual void OnNodeAddedToWorld(Node* node){
		// auto interestedInType = &Node::StaticClass();
		// auto cl = &node->GetClass();

		// if (cl == interestedInType || cl->GetChildClasses().count(interestedInType)) {

		//}
	};

public:
	virtual void Update();

	virtual void ActiveCameraResize(){};
};
