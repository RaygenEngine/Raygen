#pragma once

#include "platform/Window.h"


class Platform {
	friend class Engine_;


	static void Init(WindowCreationParams mainWindowParams = {});
	static void Destroy();

public:
	[[nodiscard]] static Window* GetMainWindow();
	[[nodiscard]] static GLFWwindow* GetMainHandle() { return GetMainWindow()->GetHandle(); }
	[[nodiscard]] static glm::uvec2 GetMainSize() { return GetMainWindow()->GetSize(); }
	[[nodiscard]] static HWND GetMainNativeHandle() { return GetMainWindow()->GetNativeHandle(); }

	static void PollEvents();
};
