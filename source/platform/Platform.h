#pragma once

#include "platform/Window.h"


class Platform {
	friend class Engine_;


	static void Init(WindowCreationParams mainWindowParams = {});
	static void Destroy();

public:
	static std::vector<const char*> GetVulkanExtensions();

	[[nodiscard]] static Window* GetMainWindow();
	[[nodiscard]] static GLFWwindow* GetMainHandle() { return GetMainWindow()->GetHandle(); }
	[[nodiscard]] static glm::uvec2 GetMainSize() { return GetMainWindow()->GetSize(); }

	static void PollEvents();
};
