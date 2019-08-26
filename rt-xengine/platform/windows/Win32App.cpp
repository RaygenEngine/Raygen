#include "pch.h"

#include "platform/windows/Win32App.h"
#include "platform/windows/TranslateWin32VirtualKeys.h"
#include "renderer/renderers/opengl/test/GLTestRenderer.h"

using namespace Renderer::OpenGL;

namespace Platform
{
	bool Win32App::CreateEngine(const std::string& applicationPath, const std::string& dataDirName)
	{
		m_engine = std::make_unique<System::Engine>();
		return m_engine->InitDirectories(applicationPath, dataDirName);
	}

	bool Win32App::CreateWorldFromFile(const std::string& filePath)
	{
		return m_engine->CreateWorldFromFile(filePath);
	}

	bool Win32App::StartRenderer(System::RendererRegistrationIndex index)
	{
		// in order of appropriate short-circuiting
		return m_engine->SwitchRenderer(index) &&
			m_engine->GetRenderer()->InitRendering(m_window->GetHWND(), m_window->GetHInstance()) &&
			m_engine->GetRenderer()->InitScene(m_window->GetWidth(), m_window->GetHeight());
	}

	bool Win32App::CreateWin32Window(
		UINT style,
		LPCSTR name,
		HBRUSH backgroundBrushColor,
		LPCSTR cursorName,
		int32 xpos,
		int32 ypox,
		LONG cstyle,
		LPCSTR title,
		int32 width,
		int32 height)
	{
		HINSTANCE hInstance = GetModuleHandle(nullptr);

		// attach engine to window
		m_window = std::make_unique<Win32Window>();

		RT_XENGINE_ASSERT_RETURN_FALSE(m_window->Register(
			style,
			name,
			backgroundBrushColor,
			LoadCursor(NULL, cursorName),
			hInstance), "Failed to register application window!");

		RT_XENGINE_ASSERT_RETURN_FALSE(m_window->Create(xpos, ypox, width, height, title, cstyle), "Failed to create application window!");

		// pass App pointer inside the hwnd userdata (minor hack for app callbacks)
		SetWindowLongPtr(m_window->GetHWND(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		return true;
	}

#ifdef XINPUT_CONTROLLER_ENABLED

#include <Xinput.h>

	static int32 FindAvailableController()
	{
		DWORD controllerIndex = 0;
		for (; controllerIndex < XUSER_MAX_COUNT; controllerIndex++)
		{
			XINPUT_STATE state;
			ZeroMemory(&state, sizeof(XINPUT_STATE));

			if (XInputGetState(controllerIndex, &state) == ERROR_SUCCESS)
			{
				RT_XENGINE_LOG_INFO("Found active XInput controller, index: {}", controllerIndex);
				break;
			}
		}

		return controllerIndex;
	}

	void GenerateXInputControllerMessages(HWND hWnd)
	{
		// send changes directly to engine input, don't post messages its slow
		auto& input = reinterpret_cast<Win32App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA))->GetEngine()->GetInput();

		// first active controller index
		static DWORD controllerIndex = FindAvailableController();

		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		XInputGetState(controllerIndex, &state);

		const auto CalculateThumb = [](const glm::vec2& point, int32 deadZone)
		{
			Input::Thumb thumb{};

			const float magnitude = glm::length(point);
			thumb.direction = glm::normalize(point);

			// clamp and normalize
			magnitude > deadZone ? thumb.magnitude = (glm::min(magnitude, 32767.f) - deadZone) / (32767 - deadZone) : thumb.magnitude = 0.0f;

			return thumb;
		};

		Input::AnalogState analogState{};

		analogState.thumbL = CalculateThumb({ state.Gamepad.sThumbLX , state.Gamepad.sThumbLY }, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		analogState.thumbR = CalculateThumb({ state.Gamepad.sThumbRX , state.Gamepad.sThumbRY }, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

		const auto CalculateTrigger = [](BYTE magnitude, int32 deadZone)
		{
			return magnitude > deadZone ? (glm::min(static_cast<float>(magnitude), 255.f) - deadZone) / (255.f - deadZone) : 0.f;
		};

		analogState.triggerL = CalculateTrigger(state.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
		analogState.triggerR = CalculateTrigger(state.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

		input.UpdateAnalogState(analogState);

		// check for keystrokes
		XINPUT_KEYSTROKE keystroke;
		ZeroMemory(&keystroke, sizeof(XINPUT_KEYSTROKE));
		XInputGetKeystroke(controllerIndex, {}, &keystroke);

		UINT msg{};

		switch (keystroke.Flags)
		{
			// we only handle the keydown and keyup events, repeat is handled internally by the engine 
		case XINPUT_KEYSTROKE_KEYDOWN:
			input.UpdateKeyPressed(TranslateXInputVirtualKeys(keystroke.VirtualKey));
			break;

		case XINPUT_KEYSTROKE_KEYUP:
			input.UpdateKeyReleased(TranslateXInputVirtualKeys(keystroke.VirtualKey));
			break;

		case XINPUT_KEYSTROKE_REPEAT:
		default:;
			// if no keystrokes, repeat is handled in engine - don't send repeat messages
		}
	}

#endif

	int32 Win32App::MainLoop()
	{
		// Register one or more renderers (more than one requires switching logic)
		auto glTestRendererHandle = RegisterRenderer<GLTestRenderer>();

		auto currentRenderer = glTestRendererHandle;

		// Start the renderer
		if (!StartRenderer(currentRenderer))
		{
			RT_XENGINE_LOG_FATAL("Failed to create Renderer!");
			return -1;
		}

		// Unload disk assets before starting main loop
		// m_engine->UnloadDiskAssets();

		// Display window before starting main loop
		m_window->Display();

		MSG msg;
		while (!GetWindow()->IsClosed())
		{
			// clear input soft state (pressed keys, etc.)
			m_engine->GetInput().ClearSoftState();

#ifdef XINPUT_CONTROLLER_ENABLED
			GenerateXInputControllerMessages(m_window->GetHWND());
#endif

			while (::PeekMessage(&msg, m_window->GetHWND(), 0, 0, PM_REMOVE) > 0)
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);

				// Basic registered renderer switching logic
				//if (msg.message == WM_KEYDOWN && msg.wParam == VK_TAB)
				//{
				//	currentRenderer = currentRenderer == glTestRendererHandle ? optixTestRendererHandle : glTestRendererHandle;
				//	// Start renderer
				//	if (!StartRenderer(currentRenderer))
				//	{
				//		RT_XENGINE_LOG_FATAL("Failed to create Renderer!");
				//		return -1;
				//	}
				//}
			}
			// update world 
			m_engine->GetWorld()->Update();
			// update renderer (also checks world updates, eg. camera/ entity moved, light color changed)
			m_engine->GetRenderer()->Update();
			// render
			m_engine->GetRenderer()->Render();
		}

		return static_cast<int32>(msg.wParam);
	}
}
