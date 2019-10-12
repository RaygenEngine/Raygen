#pragma once

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
	virtual bool InitRendering(HWND assochWnd, HINSTANCE instance) = 0;

	// Init Scene (shaders/ upload stuff etc.);
	virtual bool InitScene() = 0;

	virtual void Update() = 0;

	virtual void Render() = 0;

	virtual void SwapBuffers() = 0;

	virtual bool SupportsEditor() = 0;
};
