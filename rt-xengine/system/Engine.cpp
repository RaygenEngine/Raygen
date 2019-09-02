#include "pch.h"

#include "system/Engine.h"
#include "assets/other/xml/XMLDoc.h"
#include "assets/DiskAssetManager.h"
#include "world/World.h"
#include "renderer/Renderer.h"
#include "world/NodeFactory.h"
#include "editor/Editor.h"

namespace System
{
	Engine::Engine()
	{
	}

	Engine::~Engine()
	{
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
		const auto sceneXML = m_diskAssetManager->LoadXMLDocAsset(filename);

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
	
	bool Engine::ShouldUpdateWorld() const
	{
		if (m_editor.get())
		{
			return m_editor->ShouldUpdateWorld();
		}
		return true;
	}
	
	bool Engine::IsUsingEditor() const
	{
		// TODO: in the future disable editor with this flag.
		return m_editor.get();
	}
	
	void Engine::InitEditor()
	{
		m_editor = std::make_unique<Editor::Editor>(this);
	}
}
