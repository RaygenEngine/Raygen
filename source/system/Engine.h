#pragma once
struct GLFWwindow;
using WindowType = GLFWwindow;

class AssetImporterManager;
class AssetFrontEndManager;
class Renderer;
class Window;
class World;
struct Input;
class Editor;

class AppBase;

class NodeFactory;

class VkSampleRenderer;

using RendererT = VkSampleRenderer;

#include "system/Timer.h"

inline struct ViewportCoordinates {
	glm::uvec2 position{};
	glm::uvec2 size{};
	bool operator==(const ViewportCoordinates&) const = default;
} g_ViewportCoordinates;


class Engine {
private:
	Engine() = default;

public:
	struct DrawReporter {

		uint64 tris{ 0ull };
		uint64 draws{ 0ull };

		void Reset();
	};

public:
	[[nodiscard]] static Engine& Get()
	{
		static Engine instance;
		return instance;
	}

	bool m_remakeWindow{ true };

	// Not guaranteed to exist at all times.
	[[nodiscard]] static World* GetWorld() { return Get().m_world; }

	[[nodiscard]] static WindowType* GetMainWindow() { return Get().m_window; }

	[[nodiscard]] static AssetImporterManager* GetAssetImporterManager() { return Get().m_assetImporterManager; }

	[[nodiscard]] static AssetFrontEndManager* GetAssetFrontEndManager() { return Get().m_assetFrontEndManager; }

	// Input will be valid forever after initialization.
	[[nodiscard]] static Input& GetInput() { return *Get().m_input; }

	[[nodiscard]] static Editor* GetEditor() { return Get().m_editor; }

	[[nodiscard]] static DrawReporter* GetDrawReporter() { return &Get().m_drawReporter; }

	~Engine();

	Engine(Engine const&) = delete;
	Engine(Engine&&) = delete;

	Engine& operator=(Engine const&) = delete;
	Engine& operator=(Engine&&) = delete;

private:
	DrawReporter m_drawReporter;

	// Owning Pointer, Expected to be valid 'forever' after InitEngine.
	AssetImporterManager* m_assetImporterManager{ nullptr };

	// Owning Pointer, Expected to be valid 'forever' after InitEngine.
	AssetFrontEndManager* m_assetFrontEndManager{ nullptr };


	// Owning Pointer, Expected to be valid 'forever' after InitEngine at this time.
	WindowType* m_window{ nullptr };

	// Owning Pointer, Expected to be valid 'forever' after InitEngine.
	Input* m_input{ nullptr };

	// Owning Pointer. No guarantees can be made for world pointer.
	// It may be invalidated during runtime when loading other worlds etc.
	World* m_world{ nullptr };

	// Non owning pointer, expected to be valid for the whole program execution
	AppBase* m_app{ nullptr };

	Editor* m_editor{ nullptr };

	timer::Timer m_initToFrameTimer{};
	timer::Timer m_frameTimer{ true };

	bool m_isEditorActive{ true };
	bool m_isEditorEnabled{ true };

	std::string m_statusLine{};
	float m_lastFrameTime{ 0.f };

	float m_steadyFps{ 0.f };
	ch::system_clock::time_point m_lastRecordTime;
	size_t m_framesSinceLastRecord{ 0 }; // WARNING: use 64 bits to avoid overflow.

	void InitRenderer();

public:
	// Init the internal engine systems.
	// You MUST run this to properly init the engine
	//
	// Expects a non-owning pointer to the external App object.
	//
	void InitEngine(AppBase* app);

	void CreateWorldFromFile(const std::string& filename);

	// Avoid this if possible and always refactor cmd debug features to normal features.
	static bool HasCmdArgument(const std::string& argument);

	[[nodiscard]] static bool ShouldUpdateWorld();

	// Query if the editor is currently active. Can be changed any frame
	[[nodiscard]] static bool IsEditorActive();

	// Query if the editor could be activated during runtime. Cannot change after engine initialization
	[[nodiscard]] static bool IsEditorEnabled();

	static void SetStatusLine(const std::string& newLine) { Get().m_statusLine = newLine; }
	[[nodiscard]] static std::string GetStatusLine() { return Get().m_statusLine; }

	// Not completely accurate, for now its just 1 / lastFrameTime.
	// Completely independant of world timers.
	[[nodiscard]] static float GetFPS();

	void ReportFrameDrawn();

	void ToggleEditor();
	void ActivateEditor();
	void DeactivateEditor();


	void DeinitEngine();
};
