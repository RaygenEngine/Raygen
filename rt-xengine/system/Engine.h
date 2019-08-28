#pragma once

#include "input/Input.h"

namespace Renderer
{
	class Renderer;
}

namespace World
{
	class World;
	class NodeFactory;
}

namespace Assets
{
	class DiskAssetManager;
}

namespace System
{
	using RendererRegistrationIndex = uint32;

	class Engine
	{
		struct RendererMetadata 
		{
			std::string name;
			std::function<Renderer::Renderer*(Engine*)> Construct;
		};

		std::vector<RendererMetadata> m_rendererRegistrations;

		std::unique_ptr<Assets::DiskAssetManager> m_diskAssetManager;

		std::unique_ptr<World::World> m_world;
		std::unique_ptr<Renderer::Renderer> m_renderer;

		Input::Input m_input;

	public:
		Engine();
		~Engine();
		
		bool InitDirectories(const std::string& applicationPath, const std::string& dataDirectoryName);

		Assets::DiskAssetManager* GetDiskAssetManager() const { return m_diskAssetManager.get(); }
		Renderer::Renderer* GetRenderer() const { return m_renderer.get(); }
		World::World* GetWorld() const { return m_world.get(); }

		bool CreateWorldFromFile(const std::string& filename, World::NodeFactory* factory);

		// if another renderer is already active, then destroy old and 
		// then activate the next
		bool SwitchRenderer(RendererRegistrationIndex registrationIndex);

		// non const ref (access from engine allows stuff like clearing of softstates, etc)
		Input::Input& GetInput() { return m_input; }

		void UnloadDiskAssets();

		template<typename RendererClass>
		RendererRegistrationIndex RegisterRenderer()
		{
			m_rendererRegistrations.push_back({ RendererClass::MetaName(), &RendererClass::MetaConstruct });

			return static_cast<RendererRegistrationIndex>(m_rendererRegistrations.size() - 1);
		}
	};
}