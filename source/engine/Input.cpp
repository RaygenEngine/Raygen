#include "pch.h"
#include "engine/Input.h"

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
