#pragma once
struct GLFWwindow;

struct WindowCreationParams {
	glm::uvec2 size{ 1920, 1080 };
	const char* title{ "Raygen" };
};

// External Window interface
class Window {
public:
	Window(WindowCreationParams params);
	~Window();

	// PERF: store window size to avoid windows API call


	[[nodiscard]] glm::uvec2 GetSize() const;
	void SetTitle(const char* title);


	[[nodiscard]] GLFWwindow* GetHandle() const { return m_window; }

private:
	GLFWwindow* m_window;
};
