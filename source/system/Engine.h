#pragma once
class Win32Window;
using WindowType = Win32Window;

class AssetManager;
class Renderer;
class Window;
class World;
class Input;
class Editor;

class AppBase;

// TODO:
class NodeFactory;

class Engine {
private:
	Engine() = default;

public:
	[[nodiscard]] static Engine& Get()
	{
		static Engine instance;
		return instance;
	}

	// Not guaranteed to exist at all times.
	// TODO: provide event guarantees for world validility.
	[[nodiscard]] static World* GetWorld() { return Get().m_world; }

	// It is HIGHLY recommended to validate this result in your code for future compatibility
	// Window will not be guaranteed to exist in the future (possible use: Headless Server)
	[[nodiscard]] static Win32Window* GetMainWindow() { return Get().m_window; }

	// FileAsset manager will be valid forever after initialization.
	[[nodiscard]] static AssetManager* GetAssetManager() { return Get().m_assetManager; }

	// Input will be valid forever after initialization.
	[[nodiscard]] static Input* GetInput() { return Get().m_input; }

	// TODO: possibly get rid of this to avoid getting renderers from random locations.
	template<typename AsRenderer = Renderer>
	[[nodiscard]] static AsRenderer* GetRenderer()
	{
		return dynamic_cast<AsRenderer*>(Get().m_renderer);
	}

	template<template<class> typename RendererObject, typename RenderT>
	[[nodiscard]] static RenderT* GetRenderer(RendererObject<RenderT>* contextRendererObject)
	{
		return GetRenderer<RenderT>();
	}

	// ALWAYS, ALWAYS check this return value. Editor may not be initialized at all in some cases.
	[[nodiscard]] static Editor* GetEditor() { return Get().m_editor; }

public:
	~Engine();

	Engine(Engine const&) = delete;
	Engine(Engine&&) = delete;

	Engine& operator=(Engine const&) = delete;
	Engine& operator=(Engine&&) = delete;

private:
	struct RendererMetadata {
		std::string name;
		std::function<Renderer*()> Construct;
	};

	std::vector<RendererMetadata> m_rendererRegistrations;

	// Owning Pointer, Expected to be valid 'forever' after InitEngine.
	AssetManager* m_assetManager{ nullptr };

	// Owning Pointer, Expected to be valid 'forever' after InitEngine at this time.
	WindowType* m_window{ nullptr };

	// Owning Pointer, Expected to be valid 'forever' after InitEngine.
	Input* m_input{ nullptr };

	// Owning Pointer. No guarantees can be made for world pointer.
	// It may be invalidated during runtime when loading other worlds etc.
	World* m_world{ nullptr };

	// Owning Pointer. This pointer will NEVER be valid without a valid World existing.
	// It will get invalidated when switching renderers / loading worlds.
	Renderer* m_renderer{ nullptr };

	// Non owning pointer, expected to be valid for the whole program execution
	AppBase* m_app{ nullptr };

	Editor* m_editor{ nullptr };

	Timer::DebugTimer<ch::milliseconds> m_initToFrameTimer;

public:
	// Init the internal engine systems.
	// You MUST run this to properly init the engine
	//
	// Expects a non-owning pointer to the external App object.
	//
	void InitEngine(AppBase* app);

	bool CreateWorldFromFile(const std::string& filename);

	// if another renderer is already active, then destroy old and
	// then activate the next
	void SwitchRenderer(uint32 registrationIndex);

	template<typename RendererClass>
	static uint32 RegisterRenderer()
	{
		Engine::Get().m_rendererRegistrations.push_back({ RendererClass::MetaName(), &RendererClass::MetaConstruct });
		return static_cast<uint32>(Engine::Get().m_rendererRegistrations.size() - 1);
	}

	// Avoid this if possible and always refactor cmd debug features to normal features.
	static bool HasCmdArgument(const std::string& argument);

	[[nodiscard]] bool ShouldUpdateWorld() const;
	[[nodiscard]] bool IsUsingEditor() const;

	void ReportFrameDrawn()
	{
		if (!m_initToFrameTimer.m_stopped) {
			LOG_WARN("Init to frame took: {} ms", m_initToFrameTimer.Get());
			m_initToFrameTimer.Stop();
		}
	}

	void NextRenderer()
	{
		static uint32 currentRenderer = 0;
		SwitchRenderer(++currentRenderer % m_rendererRegistrations.size());
	}
};
