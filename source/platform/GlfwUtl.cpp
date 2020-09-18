#include "pch.h"
#include "GlfwUtl.h"

#include "App.h"
#include "editor/EditorObject.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "engine/Logger.h"
#include "engine/Engine.h"

#include <imgui/imgui.h>
#include <glfw/glfw3.h>
#include <vector>

Key ToEngineKey(int32 glfwKey, Key& outSpecialKey);
Key MouseToEngineKey(int32 glfwMouse);

std::vector<const char*> glfwutl::GetVulkanExtensions()
{
	std::vector<const char*> extensions;
	uint32 size;
	const char** c_ext = glfwGetRequiredInstanceExtensions(&size);

	for (uint32 i = 0; i < size; ++i) {
		extensions.push_back(c_ext[i]);
	}

	return extensions;
}


void Error(int32 errorCode, const char* description)
{
	LOG_ERROR("GLFW Error: {} | {}", errorCode, description);
}

// PERF: Usability
// All known glfw callbacks are registered here even if not required.
// ImGui may "overwrite" some user callbacks and forwards them to us.

void WindowPositionCb(GLFWwindow* window, int32 xpos, int32 ypos) {}

void WindowSizeCb(GLFWwindow* window, int32 newWidth, int32 newHeight) {}

void WindowCloseCb(GLFWwindow* window) {}

// Is called during resize & moving and can be used to keep rendering
void WindowRefreshCb(GLFWwindow* window)
{
	Engine.GetApp()->WhileResizing();
}

void WindowFocusCb(GLFWwindow* window, int32 isFocused) // Unfortunately we can't implicit convert to bool here
{
	Event::OnWindowFocus.Broadcast(isFocused == GLFW_TRUE);
}

// aka Minimize
void WindowIconifyCb(GLFWwindow* window, int32 isIconified)
{
	Event::OnWindowMinimize.Broadcast(isIconified == GLFW_TRUE);
}

void WindowMaximizeCb(GLFWwindow* window, int32 isMaximised)
{
	Event::OnWindowMaximize.Broadcast(isMaximised == GLFW_TRUE);
}

// This should probably preferred for "resize" over window size, check glfw docs.
void WindowFramebufferSizeCb(GLFWwindow* window, int32 newWidth, int32 newHeight)
{
	if (newWidth != 0 && newHeight != 0) {
		Event::OnWindowResize.Broadcast(newWidth, newHeight);
	}
}

void WindowContentScaleCb(GLFWwindow* window, float newXScale, float newYScale) {}

void WindowMouseButtonCb(GLFWwindow* window, int32 button, int32 action, int32 modifiers)
{
	if (ImGui::GetCurrentContext()) {
		if (ImGui::GetIO().WantCaptureMouse) {
			if (action == GLFW_RELEASE) {
				Input.Z_FocusLostKeyRelease(MouseToEngineKey(button), Key::None);
			}
			return;
		}
	}

	if (EditorObject_::EditorHandleKeyEvent(button, action, -1, modifiers)) {
		return;
	}

	switch (action) {
		case GLFW_PRESS: Input.Z_UpdateKeyPressed(MouseToEngineKey(button), Key::None); break;
		case GLFW_RELEASE: Input.Z_UpdateKeyReleased(MouseToEngineKey(button), Key::None); break;
	}
}

void WindowCursorPositionCb(GLFWwindow* window, double xcoord, double ycoord)
{
	Input.Z_UpdateMouseMove({ xcoord, ycoord });
}

void WindowCursorEnterCb(GLFWwindow* window, int32 hasJustEntered) {}

void WindowScrollCb(GLFWwindow* window, double xoffset, double yoffset)
{
	if (ImGui::GetCurrentContext()) {
		if (ImGui::GetIO().WantCaptureMouse) {
			return;
		}
	}
	// FIXME: Possible bug on other platforms due to rounding
	Input.Z_UpdateScrollWheel(static_cast<int32>(yoffset));
}

void WindowKeyCb(GLFWwindow* window, int32 key, int32 scancode, int32 action, int32 modifiers)
{
	if (ImGui::GetCurrentContext()) {
		if (ImGui::GetIO().WantCaptureKeyboard) {
			return;
		}
	}

	if (EditorObject_::EditorHandleKeyEvent(key, scancode, action, modifiers)) {
		return;
	}

	//
	Key specialKey{};
	Key engineKey = ToEngineKey(key, specialKey);
	switch (action) {
		case GLFW_PRESS: Input.Z_UpdateKeyPressed(engineKey, specialKey); break;
		case GLFW_RELEASE: Input.Z_UpdateKeyReleased(engineKey, specialKey); break;
	}
}

void WindowCharacterCb(GLFWwindow* window, uint32 unicodeCharacter) {}

// Drag 'n' drop on top of window
void WindowPathDropCb(GLFWwindow* window, int32 elementCount, const char* paths[])
{
	std::vector<fs::path> strPaths;
	for (int32 i = 0; i < elementCount; i++) {
		strPaths.push_back(paths[i]);
	}

	if (auto editor = EditorObject; editor) {
		editor->OnFileDrop(std::move(strPaths));
	}
}

void MonitorConfigCb(GLFWmonitor* monitor, int32 glfwEvent)
{
	// glfwEvent may be GLFW_CONNECTED or GLFW_DISCONNECTED
}

void JoystickConnectionCb(int32 joystickId, int32 glfwEvent)
{
	LOG_REPORT("Joystick connected!");
	// glfwEvent may be GLFW_CONNECTED or GLFW_DISCONNECTED
}

void glfwutl::SetupEventCallbacks(GLFWwindow* window)
{
	// PERF: keep disabled the unused ones
	// glfwSetWindowPosCallback(window, WindowPositionCb);
	// glfwSetWindowSizeCallback(window, WindowSizeCb);
	// glfwSetWindowCloseCallback(window, WindowCloseCb);
	glfwSetWindowRefreshCallback(window, WindowRefreshCb);

	glfwSetWindowFocusCallback(window, WindowFocusCb);
	glfwSetWindowIconifyCallback(window, WindowIconifyCb);
	glfwSetWindowMaximizeCallback(window, WindowMaximizeCb);
	glfwSetFramebufferSizeCallback(window, WindowFramebufferSizeCb);
	// glfwSetWindowContentScaleCallback(window, WindowContentScaleCb);
	glfwSetMouseButtonCallback(window, WindowMouseButtonCb);
	glfwSetCursorPosCallback(window, WindowCursorPositionCb);
	// glfwSetCursorEnterCallback(window, WindowCursorEnterCb);
	glfwSetScrollCallback(window, WindowScrollCb);
	glfwSetKeyCallback(window, WindowKeyCb);
	// glfwSetCharCallback(window, WindowCharacterCb);
	glfwSetDropCallback(window, WindowPathDropCb);
	// glfwSetMonitorCallback(MonitorConfigCb);
	// glfwSetJoystickCallback(JoystickConnectionCb);
}

Key ToEngineKey(int32 glfwKey, Key& outSpecialKey)
{
	outSpecialKey = Key::None;
	switch (glfwKey) {
		case GLFW_KEY_UNKNOWN: return Key::Unknown;
		case GLFW_KEY_SPACE: return Key::Space;
		case GLFW_KEY_APOSTROPHE: return Key::Apostrophe;
		case GLFW_KEY_COMMA: return Key::Comma;
		case GLFW_KEY_MINUS: return Key::Minus;
		case GLFW_KEY_PERIOD: return Key::Period;
		case GLFW_KEY_SLASH: return Key::Slash;
		case GLFW_KEY_0: return Key::N0;
		case GLFW_KEY_1: return Key::N1;
		case GLFW_KEY_2: return Key::N2;
		case GLFW_KEY_3: return Key::N3;
		case GLFW_KEY_4: return Key::N4;
		case GLFW_KEY_5: return Key::N5;
		case GLFW_KEY_6: return Key::N6;
		case GLFW_KEY_7: return Key::N7;
		case GLFW_KEY_8: return Key::N8;
		case GLFW_KEY_9: return Key::N9;
		case GLFW_KEY_SEMICOLON: return Key::Semicolon;
		case GLFW_KEY_EQUAL: return Key::Equal;
		case GLFW_KEY_A: return Key::A;
		case GLFW_KEY_B: return Key::B;
		case GLFW_KEY_C: return Key::C;
		case GLFW_KEY_D: return Key::D;
		case GLFW_KEY_E: return Key::E;
		case GLFW_KEY_F: return Key::F;
		case GLFW_KEY_G: return Key::G;
		case GLFW_KEY_H: return Key::H;
		case GLFW_KEY_I: return Key::I;
		case GLFW_KEY_J: return Key::J;
		case GLFW_KEY_K: return Key::K;
		case GLFW_KEY_L: return Key::L;
		case GLFW_KEY_M: return Key::M;
		case GLFW_KEY_N: return Key::N;
		case GLFW_KEY_O: return Key::O;
		case GLFW_KEY_P: return Key::P;
		case GLFW_KEY_Q: return Key::Q;
		case GLFW_KEY_R: return Key::R;
		case GLFW_KEY_S: return Key::S;
		case GLFW_KEY_T: return Key::T;
		case GLFW_KEY_U: return Key::U;
		case GLFW_KEY_V: return Key::V;
		case GLFW_KEY_W: return Key::W;
		case GLFW_KEY_X: return Key::X;
		case GLFW_KEY_Y: return Key::Y;
		case GLFW_KEY_Z: return Key::Z;
		case GLFW_KEY_LEFT_BRACKET: return Key::LeftBracket;
		case GLFW_KEY_BACKSLASH: return Key::RightBracket;
		case GLFW_KEY_RIGHT_BRACKET: return Key::Backslash;
		case GLFW_KEY_GRAVE_ACCENT: return Key::Tilde;
		case GLFW_KEY_WORLD_1: return Key::World1;
		case GLFW_KEY_WORLD_2: return Key::World2;
		case GLFW_KEY_ESCAPE: return Key::Escape;
		case GLFW_KEY_ENTER: outSpecialKey = Key::Enter; return Key::Enter_Text;
		case GLFW_KEY_TAB: return Key::Tab;
		case GLFW_KEY_BACKSPACE: return Key::Backspace;
		case GLFW_KEY_INSERT: return Key::Insert;
		case GLFW_KEY_DELETE: return Key::Delete;
		case GLFW_KEY_RIGHT: return Key::Right;
		case GLFW_KEY_LEFT: return Key::Left;
		case GLFW_KEY_DOWN: return Key::Down;
		case GLFW_KEY_UP: return Key::Up;
		case GLFW_KEY_PAGE_UP: return Key::PageUp;
		case GLFW_KEY_PAGE_DOWN: return Key::PageDown;
		case GLFW_KEY_HOME: return Key::Home;
		case GLFW_KEY_END: return Key::End;
		case GLFW_KEY_CAPS_LOCK: return Key::CapsLock;
		case GLFW_KEY_SCROLL_LOCK: return Key::ScrollLock;
		case GLFW_KEY_NUM_LOCK: return Key::NumLock;
		case GLFW_KEY_PRINT_SCREEN: return Key::PrintScreen;
		case GLFW_KEY_PAUSE: return Key::Pause;
		case GLFW_KEY_F1: return Key::F1;
		case GLFW_KEY_F2: return Key::F2;
		case GLFW_KEY_F3: return Key::F3;
		case GLFW_KEY_F4: return Key::F4;
		case GLFW_KEY_F5: return Key::F5;
		case GLFW_KEY_F6: return Key::F6;
		case GLFW_KEY_F7: return Key::F7;
		case GLFW_KEY_F8: return Key::F8;
		case GLFW_KEY_F9: return Key::F9;
		case GLFW_KEY_F10: return Key::F10;
		case GLFW_KEY_F11: return Key::F11;
		case GLFW_KEY_F12: return Key::F12;
		case GLFW_KEY_F13: return Key::F13;
		case GLFW_KEY_F14: return Key::F14;
		case GLFW_KEY_F15: return Key::F15;
		case GLFW_KEY_F16: return Key::F16;
		case GLFW_KEY_F17: return Key::F17;
		case GLFW_KEY_F18: return Key::F18;
		case GLFW_KEY_F19: return Key::F19;
		case GLFW_KEY_F20: return Key::F20;
		case GLFW_KEY_F21: return Key::F21;
		case GLFW_KEY_F22: return Key::F22;
		case GLFW_KEY_F23: return Key::F23;
		case GLFW_KEY_F24: return Key::F24;
		case GLFW_KEY_F25: return Key::F25;
		case GLFW_KEY_KP_0: return Key::Num_0;
		case GLFW_KEY_KP_1: return Key::Num_1;
		case GLFW_KEY_KP_2: return Key::Num_2;
		case GLFW_KEY_KP_3: return Key::Num_3;
		case GLFW_KEY_KP_4: return Key::Num_4;
		case GLFW_KEY_KP_5: return Key::Num_5;
		case GLFW_KEY_KP_6: return Key::Num_6;
		case GLFW_KEY_KP_7: return Key::Num_7;
		case GLFW_KEY_KP_8: return Key::Num_8;
		case GLFW_KEY_KP_9: return Key::Num_9;
		case GLFW_KEY_KP_DECIMAL: return Key::Num_Comma;
		case GLFW_KEY_KP_DIVIDE: return Key::Num_Divide;
		case GLFW_KEY_KP_MULTIPLY: return Key::Num_Multiply;
		case GLFW_KEY_KP_SUBTRACT: return Key::Num_Subtract;
		case GLFW_KEY_KP_ADD: return Key::Num_Add;
		case GLFW_KEY_KP_ENTER: return Key::Num_Enter;
		case GLFW_KEY_KP_EQUAL: return Key::Num_Equal;
		case GLFW_KEY_LEFT_SHIFT: outSpecialKey = Key::Shift; return Key::LeftShift;
		case GLFW_KEY_LEFT_CONTROL: outSpecialKey = Key::Ctrl; return Key::LeftCtrl;
		case GLFW_KEY_LEFT_ALT: outSpecialKey = Key::Alt; return Key::LeftAlt;
		case GLFW_KEY_LEFT_SUPER: outSpecialKey = Key::Platform; return Key::LeftPlatform;
		case GLFW_KEY_RIGHT_SHIFT: outSpecialKey = Key::Shift; return Key::RightShift;
		case GLFW_KEY_RIGHT_CONTROL: outSpecialKey = Key::Ctrl; return Key::RightCtrl;
		case GLFW_KEY_RIGHT_ALT: outSpecialKey = Key::Alt; return Key::RightAlt;
		case GLFW_KEY_RIGHT_SUPER: outSpecialKey = Key::Platform; return Key::RightPlatform;
		case GLFW_KEY_MENU: return Key::Menu;
	}
	return Key::Unknown;
}

Key MouseToEngineKey(int32 glfwMouse)
{
	switch (glfwMouse) {
		case GLFW_MOUSE_BUTTON_LEFT: return Key::Mouse_LeftClick;
		case GLFW_MOUSE_BUTTON_RIGHT: return Key::Mouse_RightClick;
		case GLFW_MOUSE_BUTTON_MIDDLE: return Key::Mouse_MiddleClick;
		case GLFW_MOUSE_BUTTON_4: return Key::Mouse_Back;
		case GLFW_MOUSE_BUTTON_5: return Key::Mouse_Forward;
		case GLFW_MOUSE_BUTTON_6: return Key::Mouse_6;
		case GLFW_MOUSE_BUTTON_7: return Key::Mouse_7;
		case GLFW_MOUSE_BUTTON_8: return Key::Mouse_8;
	}
	return Key::Unknown;
}
