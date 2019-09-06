#pragma once

#include "renderer/NodeObserver.h"
#include "system/EngineComponent.h"
#include <unordered_set> // REFACT:
// For sub-renderer registration
// TODO: Add static assert for missing constructor with engine as parameter.
#define MAKE_METADATA(Class) \
	public:\
	    static Renderer* MetaConstruct(Engine* context) \
		{ \
	         return new Class(context); \
	    }\
	    static std::string MetaName() \
		{ \
	        return std::string(#Class); \
	    } \

class Renderer : public EngineComponent
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

	// TODO:
	AssetManager* GetAssetManager() const;
public:
	Renderer::Renderer(Engine* engine)
		: EngineComponent(engine) {}

	virtual Renderer::~Renderer() {}

	// Windows based init rendering (implement in "context"-base renderers)
	virtual bool InitRendering(HWND assochWnd, HINSTANCE instance);

	// Init Scene (shaders/ upload stuff etc.);
	virtual bool InitScene(int32 width, int32 height) = 0;

	virtual void Update() = 0;

	// Render, on the render target stored by SwitchRenderTarget from InitRendering/UpdateRendering (don't pass render target
	// as Render() parameter, its not "safe") 
	virtual void Render() = 0;

	virtual void SwapBuffers() { };
};

