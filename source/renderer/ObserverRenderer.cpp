#include "pch/pch.h"

#include "renderer/ObserverRenderer.h"
#include "system/Engine.h"
#include "system/Input.h"
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
}

void ObserverRenderer::OnActiveCameraChanged(CameraNode* node)
{
	m_activeCamera = Engine::GetWorld()->GetActiveCamera();
	if (m_activeCamera) {
		ActiveCameraResize();
	}
}

void ObserverRenderer::Update()
{
	if (m_activeCamera && m_activeCamera->GetDirtyFlagset()[CameraNode::DF::ViewportSize]) {
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

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::OEM_PLUS)) {
		m_gamma += 0.03f;
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::OEM_MINUS)) {
		m_gamma -= 0.03f;
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::MULTIPLY)) {
		m_exposure += 0.03f;
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::DIVIDE)) {
		m_exposure -= 0.03f;
	}
}
