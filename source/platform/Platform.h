#pragma once

#include "platform/Window.h"


namespace Platform {
inline Window* MainWindow{};

std::vector<const char*> GetVulkanExtensions();
} // namespace Platform
