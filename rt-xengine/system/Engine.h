#pragma once

#include "system/Input.h"
#include "platform/windows/Win32Window.h"
#include "renderer/Renderer.h"
#include "world/World.h"
#include "world/NodeFactory.h"

namespace Assets
{
	class AssetManager;
}

using RendererRegistrationIndex = uint32;

class Engine
{
public:
	using WindowType = Win32Window;

private:

	struct RendererMetadata 
	{
		std::string name;
		std::function<Renderer*(Engine*)> Construct;
	};

	std::vector<RendererMetadata> m_rendererRegistrations;


	// TODO: implement non deletable engineComponent on stack
	

	std::unique_ptr<Assets::AssetManager> m_diskAssetManager;
	std::unique_ptr<World> m_world;
	std::unique_ptr<Renderer> m_renderer;
	WindowType m_window;

	Input m_input;

public:
	Engine();
	~Engine();
		
	bool InitDirectories(const std::string& applicationPath, const std::string& dataDirectoryName);

	Assets::AssetManager* GetDiskAssetManager() const { return m_diskAssetManager.get(); }

	Renderer* GetRenderer() const { return m_renderer.get(); }
	World* GetWorld() const { return m_world.get(); }
	WindowType* GetWindow() { return &m_window; }

	// Can return nullptr
	//Editor* GetEditor() { return m_editor; }

	Input& GetInput() { return m_input; }

	bool CreateWorldFromFile(const std::string& filename, NodeFactory* factory);

	// if another renderer is already active, then destroy old and 
	// then activate the next
	bool SwitchRenderer(RendererRegistrationIndex registrationIndex);

	void UnloadDiskAssets();

	template<typename RendererClass>
	RendererRegistrationIndex RegisterRenderer()
	{
		m_rendererRegistrations.push_back({ RendererClass::MetaName(), &RendererClass::MetaConstruct });

		return static_cast<RendererRegistrationIndex>(m_rendererRegistrations.size() - 1);
	}
};

Engine::WindowType a;