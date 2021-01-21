#include "Input.h"

#include "engine/Engine.h"
#include "platform/Platform.h"

#include <glfw/glfw3.h>

void Input::ReleaseSpecialKey(Key released, Key special)
{
	Key otherKey; // The other same special key, eg if special is LShift the otherKey should become RShift

	switch (special) {
		case Key::Shift: otherKey = (released == Key::LeftShift) ? Key::RightShift : Key::LeftShift; break;
		case Key::Alt: otherKey = (released == Key::LeftAlt) ? Key::RightAlt : Key::LeftAlt; break;
		case Key::Ctrl: otherKey = (released == Key::LeftCtrl) ? Key::RightCtrl : Key::LeftCtrl; break;
		case Key::Platform: otherKey = (released == Key::LeftPlatform) ? Key::RightPlatform : Key::LeftPlatform; break;
		case Key::Enter: otherKey = (released == Key::Enter_Text) ? Key::Num_Enter : Key::Enter_Text; break;
	}

	if (!IsDown(otherKey)) {
		Z_UpdateKeyReleased(special, Key::None);
	}
}


void Input::Z_UpdateKeyPressed(Key key, Key special)
{
	keysJustPressed.insert(key);
	keyStates[static_cast<int32>(key)] = true;

	if (special != Key::None && !IsDown(special)) {
		Z_UpdateKeyPressed(special, Key::None);
	}
}

void Input::Z_UpdateKeyReleased(Key key, Key special)
{
	keysJustReleased.insert(key);
	keyStates[static_cast<int32>(key)] = false;

	if (special != Key::None) {
		ReleaseSpecialKey(key, special);
	}
}

void Input::Z_UpdateMouseMove(glm::vec2 newCoords)
{
	if (IsDown(Key::Mouse_LeftClick) || IsDown(Key::Mouse_RightClick) || IsDown(Key::Mouse_MiddleClick)) {
		isMouseDragging = true;
	}
	relativeCursorPosition = newCoords - cursorPosition;
	cursorPosition = newCoords;
}

void Input::Z_UpdateScrollWheel(int32 newDelta)
{
	scrollWheelDelta = newDelta;
}

void Input::Z_ClearFrameState()
{
	scrollWheelDelta = 0;
	keysJustPressed.clear();
	keysJustReleased.clear();
	isMouseDragging = false;
	relativeCursorPosition = {};
}

void Input::Z_FocusLostKeyRelease(Key key, Key special)
{
	if (keyStates[static_cast<int32>(key)]) {
		Z_UpdateKeyReleased(key, special);
	}
}

glm::vec2 Input::GetMouseViewportPosition() const noexcept
{
	return glm::ivec2(cursorPosition) - g_ViewportCoordinates.position;
}

bool Input::IsMouseInViewport() const noexcept
{
	auto pos = GetMouseViewportPosition();
	return pos.x >= 0 && pos.y >= 0 && pos.x < g_ViewportCoordinates.size.x && pos.y < g_ViewportCoordinates.size.y;
}

glm::vec2 Input::GetMouseViewportUV() const noexcept
{
	if (g_ViewportCoordinates.size.x == 0 || g_ViewportCoordinates.size.y == 0) {
		return {};
	}

	return glm::vec2(
		//
		GetMouseViewportPosition().x / g_ViewportCoordinates.size.x,
		GetMouseViewportPosition().y / g_ViewportCoordinates.size.y);
}

void Input::SetMousePos(glm::vec2 newPos)
{
	glfwSetCursorPos(Platform::GetMainHandle(), newPos.x, newPos.y);
}

void Input::LockMouse()
{
	glfwSetInputMode(Platform::GetMainHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Input::UnlockMouse()
{
	glfwSetInputMode(Platform::GetMainHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
