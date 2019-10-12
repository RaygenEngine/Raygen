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

class NodeFactory;

#include "system/Timer.h"

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
	[[nodiscard]] static World* GetWorld() { return Get().m_world; }

	// It is HIGHLY recommended to validate this result in your code for future compatibility
	// Window will not be guaranteed to exist in the future (possible use: Headless Server)
	[[nodiscard]] static WindowType* GetMainWindow() { return Get().m_window; }

	// FileAsset manager will be valid forever after initialization.
	[[nodiscard]] static AssetManager* GetAssetManager() { return Get().m_assetManager; }

	// Input will be valid forever after initialization.
	[[nodiscard]] static Input* GetInput() { return Get().m_input; }

	// DOC:
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

	timer::DebugTimer<ch::milliseconds> m_initToFrameTimer;

	struct RendererMetadata {
		std::string name;
		std::function<Renderer*()> Construct;
	};
	std::vector<RendererMetadata> m_rendererRegistrations;

	bool m_isEditorActive{ true };
	bool m_isEditorEnabled{ true };

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
		static_assert(std::is_base_of_v<Renderer, RendererClass>, "Attempting to register a non renderer class.");

		RendererMetadata data;
		data.Construct = []() -> Renderer* {
			return new RendererClass();
		};

		data.name = refl::GetName<RendererClass>();

		Engine::Get().m_rendererRegistrations.emplace_back(data);
		return static_cast<uint32>(Engine::Get().m_rendererRegistrations.size() - 1);
	}

	// Avoid this if possible and always refactor cmd debug features to normal features.
	static bool HasCmdArgument(const std::string& argument);

	[[nodiscard]] static bool ShouldUpdateWorld();

	// Query if the editor is currently active. Can be changed any frame
	[[nodiscard]] static bool IsEditorActive();

	// Query if the editor could be activated during runtime. Cannot change after engine initialization
	[[nodiscard]] static bool IsEditorEnabled();

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

	void ToggleEditor();
};
