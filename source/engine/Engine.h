#pragma once

#include "engine/Object.h"

struct GLFWwindow;

class AssetImporterManager;
class AssetFrontEndManager;
class Renderer;
class Window;
class World;
struct Input;
class Editor;

class App;

class NodeFactory;

#include "engine/Timer.h"

inline struct ViewportCoordinates {
	glm::uvec2 position{};
	glm::uvec2 size{ 128, 128 };
	bool operator==(const ViewportCoordinates&) const = default;
} g_ViewportCoordinates;


inline class S_Engine : public Object {

public:
	S_Engine() = default;

	// Not guaranteed to exist at all times.
	[[nodiscard]] World* GetWorld() { return m_world; }

	[[nodiscard]] GLFWwindow* GetMainWindow() { return m_window; }

	[[nodiscard]] AssetImporterManager* GetAssetImporterManager() { return m_assetImporterManager; }

	[[nodiscard]] AssetFrontEndManager* GetAssetFrontEndManager() { return m_assetFrontEndManager; }

	// Input will be valid forever after initialization.
	[[nodiscard]] Input& GetInput() { return *m_input; }

	[[nodiscard]] Editor* GetEditor() { return m_editor; }

	~S_Engine();

	S_Engine(S_Engine const&) = delete;
	S_Engine(S_Engine&&) = delete;
	S_Engine& operator=(S_Engine const&) = delete;
	S_Engine& operator=(S_Engine&&) = delete;

private:
	// Owning Pointer, Expected to be valid 'forever' after InitEngine.
	AssetImporterManager* m_assetImporterManager{ nullptr };

	// Owning Pointer, Expected to be valid 'forever' after InitEngine.
	AssetFrontEndManager* m_assetFrontEndManager{ nullptr };


	// Owning Pointer, Expected to be valid 'forever' after InitEngine at this time.
	GLFWwindow* m_window{ nullptr };

	// Owning Pointer, Expected to be valid 'forever' after InitEngine.
	Input* m_input{ nullptr };

	// Owning Pointer. No guarantees can be made for world pointer.
	// It may be invalidated during runtime when loading other worlds etc.
	World* m_world{ nullptr };

	// Non owning pointer, expected to be valid for the whole program execution
	App* m_app{ nullptr };

	Editor* m_editor{ nullptr };

	timer::Timer m_initToFrameTimer{};
	timer::Timer m_frameTimer{ true };

	float m_lastFrameTime{ 0.f };

	float m_steadyFps{ 0.f };
	ch::system_clock::time_point m_lastRecordTime;
	size_t m_framesSinceLastRecord{ 0 }; // WARNING: use 64 bits to avoid overflow.

public:
	// Init the internal engine systems.
	// You MUST run this to properly init the engine
	//
	// Expects a non-owning pointer to the external App object.
	//
	void InitEngine(App* app);

	void CreateWorldFromFile(const std::string& filename);

	// Avoid this if possible and always refactor cmd debug features to normal features.
	bool HasCmdArgument(const std::string& argument);

	[[nodiscard]] bool ShouldUpdateWorld();

	[[nodiscard]] float GetFPS();

	void ReportFrameDrawn();

	void DeinitEngine();

	[[nodiscard]] App* GetApp() const { return m_app; }

} Engine;
