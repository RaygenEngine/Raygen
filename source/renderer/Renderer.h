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

	// Windows based init rendering (implement in "context"-base renderers)
	virtual void Init(HWND assochWnd, HINSTANCE instance) = 0;

	virtual void DrawFrame() = 0;

	virtual bool SupportsEditor() = 0;
};
