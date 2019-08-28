#pragma once

#include "renderer/NodeObserver.h"

namespace Renderer
{
	// For sub-renderer registration
	#define MAKE_METADATA(Class) \
	public:\
	    static Renderer* MetaConstruct(System::Engine* context) \
		{ \
	         return new Class(context); \
	    }\
	    static std::string MetaName() \
		{ \
	        return std::string(#Class); \
	    } \

	class Renderer : public System::EngineObject
	{
		std::unordered_set<NodeObserver*> m_observers;
		std::unordered_set<NodeObserver*> m_dirtyObservers;

	protected:
		template <typename RendererType, typename ObserverType>
		std::shared_ptr<ObserverType> CreateObserver(RendererType* renderer, typename ObserverType::NT* typedNode)
		{
			std::shared_ptr<ObserverType> observer = std::shared_ptr<ObserverType>(new ObserverType(renderer, typedNode), [&](ObserverType* assetPtr)
			{
				m_observers.erase(assetPtr);
				m_dirtyObservers.erase(assetPtr);
				delete assetPtr;
			});

			m_observers.insert(observer.get());

			return observer;
		}

		bool IsObserverDirty(NodeObserver* obs) const { return m_dirtyObservers.find(obs) != m_dirtyObservers.end(); }

	public:
		Renderer(System::Engine* engine);
		virtual ~Renderer(); 

		// Windows based init rendering (implement in "context"-base renderers)
		virtual bool InitRendering(HWND assochWnd, HINSTANCE instance);

		// Init Scene (shaders/ upload stuff etc.);
		virtual bool InitScene(int32 width, int32 height) = 0;

		// Render, on the render target stored by SwitchRenderTarget from InitRendering/UpdateRendering (don't pass render target
		// as Render() parameter, its not "safe") 
		virtual void Render() = 0;

		virtual void ToString(std::ostream& os) const { os << "object-type: Renderer, id: " << GetObjectId(); }
	};
}
