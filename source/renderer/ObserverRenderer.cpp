#include "pch/pch.h"

#include "renderer/ObserverRenderer.h"
#include "system/Engine.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/World.h"

void ObserverRenderer::RemoveObserver(NodeObserverBase* ptr)
{
	m_observers.erase(std::find_if(
		begin(m_observers), end(m_observers), [ptr](auto& observerUnqPtr) { return observerUnqPtr.get() == ptr; }));
}

void ObserverRenderer::OnNodeRemovedFromWorld(Node* node)
{
	for (auto& [nodeClass, removerFunc] : m_onTypeRemoved) {
		auto thisNodeClass = &node->GetClass();

		if (thisNodeClass == nodeClass || nodeClass->GetChildClasses().count(thisNodeClass)) {
			removerFunc(node);
		}
	}
}

void ObserverRenderer::OnNodeAddedToWorld(Node* node)
{
	for (auto& [nodeClass, adderFunc] : m_onTypeAdded) {
		auto thisNodeClass = &node->GetClass();

		if (thisNodeClass == nodeClass || nodeClass->GetChildClasses().count(thisNodeClass)) {
			adderFunc(node);
		}
	}
};

void ObserverRenderer::Update()
{
	auto camera = Engine::GetWorld()->GetActiveCamera();
	if (camera && camera->GetDirtyFlagset()[CameraNode::DF::ViewportSize]) {
		ActiveCameraResize();
	}

	for (auto& observer : m_observers) {
		if (observer->baseNode) {
			auto flagset = observer->baseNode->GetDirtyFlagset();
			if (flagset.any()) {
				// LOG_INFO("Dirty Flags on {}: {}", observer->baseNode->GetName(), flagset);
				observer->DirtyNodeUpdate(flagset);
			}
		}
	}
}
