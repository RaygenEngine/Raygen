#pragma once

#include "platform/windows/Win32Window.h"
#include "system/Engine.h"

namespace Platform
{
// undef to not use XInput
#define XINPUT_CONTROLLER_ENABLED
	
	class Win32App
	{
		std::unique_ptr<System::Engine> m_engine;
		std::unique_ptr<Win32Window> m_window;

	public:

		Win32App() = default;
		~Win32App() = default;

		bool CreateEngine(const std::string& applicationPath, const std::string& dataDirName);
		bool CreateWorldFromFile(const std::string& filePath);

		bool CreateWin32Window(
			UINT style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS,
			LPCSTR name = TEXT("RTXENGINEWINDOWCLASS"),
			HBRUSH backgroundBrushColor = (HBRUSH)(COLOR_WINDOW + 1),
			LPCSTR cursorName = IDC_ARROW,
			int32 xpos = 150,
			int32 ypox = 150,
			LONG cstyle = WS_OVERLAPPEDWINDOW,
			LPCSTR title = TEXT("Test Window"),
			int32 width = 1920,
			int32 height = 1080);

		template <typename T> 
		System::RendererRegistrationIndex RegisterRenderer() { return m_engine->RegisterRenderer<T>(); }

		bool StartRenderer(System::RendererRegistrationIndex index);

		Win32Window* GetWindow() const { return m_window.get(); }
		System::Engine* GetEngine() const { return m_engine.get(); }

		int32 MainLoop();

		friend LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};

}
