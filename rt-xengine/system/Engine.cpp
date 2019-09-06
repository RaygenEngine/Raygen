#include "pch.h"

#include "system/Engine.h"
#include "assets/other/xml/XMLDoc.h"
#include "assets/AssetManager.h"
#include "world/World.h"
#include "renderer/Renderer.h"
#include "world/NodeFactory.h"
#include "platform/windows/Win32Window.h"

Engine::Engine()
	: m_assetManager(nullptr),
	m_world(nullptr),
	m_renderer(nullptr),
	m_window(nullptr),
	m_input(nullptr)
{
	m_input = new Input();
	m_assetManager = new AssetManager();

}

Engine::~Engine()
{			
	delete m_assetManager;
	delete m_input; 
	delete m_window;

	if (m_world) delete m_world;
	if (m_renderer) delete m_renderer;
}

bool Engine::InitDirectories(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	return m_assetManager->Init(applicationPath, dataDirectoryName);
}

bool Engine::CreateWorldFromFile(const std::string& filename, NodeFactory* factory)
{
	m_world = new World(factory);

	// load scene file
	const auto sceneXML = m_assetManager->LoadXMLDocAsset(filename);

	return m_world->LoadAndPrepareWorldFromXML(sceneXML.get());
}

bool Engine::SwitchRenderer(uint32 registrationIndex)
{
	if (registrationIndex < 0 || registrationIndex >= m_rendererRegistrations.size()) 
	{
		LOG_WARN("Attempted to switch to incorrect renderer index: {} of total registered: {}", registrationIndex, m_rendererRegistrations.size());
		return false;
	}

	if (m_renderer) {
		delete m_renderer;
	}
	m_renderer = m_rendererRegistrations[registrationIndex].Construct();

	return m_renderer;
}

void Engine::UnloadDiskAssets()
{
	m_assetManager->UnloadAssets();
}
