#include "pch.h"

#include "renderer/Renderer.h"

namespace Renderer
{
	Renderer::Renderer(System::Engine* context)
		: EngineObject(context)
	{
		RT_XENGINE_LOG_INFO("Created Renderer, id: {}", GetObjectId());
	}

	Renderer::~Renderer()
	{
		RT_XENGINE_LOG_INFO("Destroyed Renderer, id: {}", GetObjectId());
	}

	bool Renderer::InitRendering(HWND assochWnd, HINSTANCE instance)
	{
		RT_XENGINE_LOG_FATAL("This renderer does not have a windows-based rendering functionality!");
		return false;
	}

	//void Renderer::UpdateRendering()
	//{
	//	for (auto* obs : m_observers)
	//	{
	//		obs->UpdateFromVisual(m_renderTarget);
	//		m_dirtyObservers.insert(obs);
	//	}
	//}

	//bool Renderer::SwitchRenderTarget(RenderTarget* newTarget)
	//{
	//	//if (newTarget->type != GetRequiredSurfaceTargetType())
	//	//	return false;

	//	//m_renderTarget = newTarget;
	//	//return true;
	//	return true;
	//}

	//void Renderer::OnPreRender(const CTickEvent& event)
	//{
	//	// update from dirty nodes
	//	auto dirtyNodes = GetWorld()->GetDirtyNodes();

	//	for (auto* observer : m_observers)
	//	{
	//		if (dirtyNodes.find(observer->GetNodeBase()) != dirtyNodes.end())
	//		{
	//			observer->UpdateFromNode();
	//			m_dirtyObservers.insert(observer);
	//		}
	//	}
	//}

	//void Renderer::OnPostRender(const CTickEvent& event)
	//{
	//	// clear any dirty observers
	//	m_dirtyObservers.clear();
	//}
}
