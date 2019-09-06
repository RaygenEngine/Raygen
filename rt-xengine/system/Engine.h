#pragma once

class Win32Window;
using WindowType = Win32Window;

class AssetManager;
class Renderer;
class Window;
class World;
class Input;

class NodeFactory;

class Engine
{
private:
	Engine();

public:
	static Engine& Get()
	{
		static Engine instance;
		return instance;
	}
	
	[[nodiscard]] 
	static World* GetWorld()
	{
		return Get().m_world;
	}

	[[nodiscard]] 
	static Win32Window* GetWindow()
	{
		return Get().m_window;
	}

	[[nodiscard]] 
	static AssetManager* GetAssetManager()
	{
		return Get().m_assetManager;
	}

	[[nodiscard]] 
	static Renderer* GetRenderer()
	{
		return Get().m_renderer;
	}
	
	[[nodiscard]] 
	static Input* GetInput()
	{
		return Get().m_input;
	}


public:
	Engine(Engine const&) = delete;
	void operator=(Engine const&) = delete;
	
	~Engine();

private:

	struct RendererMetadata 
	{
		std::string name;
		std::function<Renderer*()> Construct;
	};

	std::vector<RendererMetadata> m_rendererRegistrations;


	// TODO: implement non deletable engineComponent on stack
	
	AssetManager* m_assetManager;
	World* m_world;
	Renderer* m_renderer;
	WindowType* m_window;
	Input* m_input;

public:
	bool InitDirectories(const std::string& applicationPath, const std::string& dataDirectoryName);

	bool CreateWorldFromFile(const std::string& filename, NodeFactory* factory);

	// if another renderer is already active, then destroy old and 
	// then activate the next
	bool SwitchRenderer(uint32 registrationIndex);

	void UnloadDiskAssets();

	template<typename RendererClass>
	uint32 RegisterRenderer()
	{
		m_rendererRegistrations.push_back({ RendererClass::MetaName(), &RendererClass::MetaConstruct });
		return static_cast<uint32>(m_rendererRegistrations.size() - 1);
	}
};
