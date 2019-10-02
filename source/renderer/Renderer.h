#pragma once

#include "renderer/NodeObserver.h"

#include <unordered_set>
#include <type_traits>


// For sub-renderer registration
#define MAKE_METADATA(Class) \
	public:\
	    static Renderer* MetaConstruct() \
		{ \
	         return new Class(); \
	    }\
	    static std::string MetaName() \
		{ \
	        return std::string(#Class); \
	    } \

class Renderer
{
	// TODO: updates
	//std::unordered_set<NodeObserver*> m_observers;
	//std::unordered_set<NodeObserver*> m_dirtyObservers;

protected:
	template <typename ObserverType>
	std::unique_ptr<ObserverType> CreateObserver(typename ObserverType::NT* typedNode)
	{
		//std::shared_ptr<ObserverType> observer = std::shared_ptr<ObserverType>(new ObserverType(renderer, typedNode), [&](ObserverType* assetPtr)
		//{
			//m_observers.erase(assetPtr);
			//m_dirtyObservers.erase(assetPtr);
		//	delete assetPtr;
		//});

		//m_observers.insert(observer.get());

		return std::make_unique<ObserverType>(typedNode);
	}

public:
	virtual ~Renderer() {}

	// Windows based init rendering (implement in "context"-base renderers)
	virtual bool InitRendering(HWND assochWnd, HINSTANCE instance);

	// Init Scene (shaders/ upload stuff etc.);
	virtual bool InitScene(int32 width, int32 height) = 0;

	virtual void Update() = 0;

	virtual void Render() = 0;

	virtual void SwapBuffers() { };
};

