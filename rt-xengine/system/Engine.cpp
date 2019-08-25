#include "pch.h"
#include "Engine.h"


#include "renderer/Renderer.h"
#include "world/NodeFactory.h"

namespace System
{
	Engine::Engine()
		: m_id(Core::UUIDGenerator::GenerateUUID())
	{
		RT_XENGINE_LOG_INFO("Created Engine context, id: {}", m_id);
	}

	Engine::~Engine()
	{
		RT_XENGINE_LOG_INFO("Destroyed Engine context, id: {}", m_id);
	}

	bool Engine::InitDirectories(const std::string& applicationPath, const std::string& dataDirectoryName)
	{
		m_diskAssetManager = std::make_unique<Assets::DiskAssetManager>(this);
		return m_diskAssetManager->Init(applicationPath, dataDirectoryName);
	}

	bool Engine::CreateWorldFromFile(const std::string& filename, World::NodeFactory* factory)
	{
		m_world = std::make_unique<World::World>(this, factory);

		// load scene file
		const auto sceneXML = m_diskAssetManager->LoadFileAsset<Assets::XMLDoc>(filename);

		return m_world->LoadAndPrepareWorldFromXML(sceneXML.get());
	}

	bool Engine::SwitchRenderer(RendererRegistrationIndex registrationIndex)
	{
		if (registrationIndex < 0 || registrationIndex >= m_rendererRegistrations.size()) 
		{
			RT_XENGINE_LOG_WARN("Attempted to switch to incorrect renderer index: {} of total registered: {}", registrationIndex, m_rendererRegistrations.size());
			return false;
		}

		// replacing the old renderer will destroy it
		// construct renderer
		m_renderer = std::unique_ptr<Renderer::Renderer>(m_rendererRegistrations[registrationIndex].Construct(this));

		return m_renderer.get();
	}

	void Engine::UnloadDiskAssets()
	{
		m_diskAssetManager->UnloadAssets();
	}

	void Engine::SetEventCallback(std::function<void(Event::Event&)> fn)
	{
		m_callbacks.push_back(fn);
	}

	void Engine::ProcessEvent(Event::Event& e)
	{
		//SetEventCallback([](Event::Eventb& a){});

		for (auto& callback : m_callbacks)
			callback(e);
	}
}
