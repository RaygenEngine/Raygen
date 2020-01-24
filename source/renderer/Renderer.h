#pragma once

// TODO: remove when you implement platforms
#include <windows.h>

class Renderer {
public:
	Renderer() = default;
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	virtual ~Renderer() = default;

	virtual void DrawFrame() = 0;

	virtual bool SupportsEditor() = 0;
};
