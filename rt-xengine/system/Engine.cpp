#include "pch.h"

#include "system/Engine.h"
#include "assets/other/xml/XMLDoc.h"
#include "assets/AssetManager.h"
#include "world/World.h"
#include "renderer/Renderer.h"
#include "world/NodeFactory.h"


Engine::Engine()
{
}

Engine::~Engine()
{
}

bool Engine::InitDirectories(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	m_diskAssetManager = std::make_unique<AssetManager>(this);
	return m_diskAssetManager->Init(applicationPath, dataDirectoryName);
}

bool Engine::CreateWorldFromFile(const std::string& filename, NodeFactory* factory)
{
	m_world = std::make_unique<World>(this, factory);

	// load scene file
	const auto sceneXML = m_diskAssetManager->LoadXMLDocAsset(filename);

	return m_world->LoadAndPrepareWorldFromXML(sceneXML.get());
}

bool Engine::SwitchRenderer(RendererRegistrationIndex registrationIndex)
{
	if (registrationIndex < 0 || registrationIndex >= m_rendererRegistrations.size()) 
	{
		LOG_WARN("Attempted to switch to incorrect renderer index: {} of total registered: {}", registrationIndex, m_rendererRegistrations.size());
		return false;
	}

	// replacing the old renderer will destroy it
	// construct renderer
	m_renderer = std::unique_ptr<Renderer>(m_rendererRegistrations[registrationIndex].Construct(this));

	return m_renderer.get();
}

void Engine::UnloadDiskAssets()
{
	m_diskAssetManager->UnloadAssets();
}
