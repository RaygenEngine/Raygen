#pragma once
#include "engine/Engine.h"

class GLFWwindow;

// External Window interface
class Window {
public:
	Window(glm::vec2 size = { 1920, 1080 }, const char* title = "Raygen");
	~Window();

	// PERF: store window size to avoid windows API call


	[[nodiscard]] glm::uvec2 GetSize() const;
	void SetTitle(const char* title);


	[[nodiscard]] GLFWwindow* GetHandle() const { return m_window; }

	struct GlfwContext {
		GlfwContext();
		~GlfwContext();
	};

private:
	GLFWwindow* m_window;
	std::shared_ptr<GlfwContext> m_glfwContext;
};
