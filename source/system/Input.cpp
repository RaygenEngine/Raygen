#include "pch/pch.h"

#include "system/Input.h"

Input::Input()
	: m_doubleClicked(false)
	, m_cursorDragged(false)
	, m_cursorPosition()
	, m_cursorRelativePosition()
	, m_wheelDelta(0)
{
}

void Input::UpdateKeyPressed(XVirtualKey key)
{
	m_keysPressed.insert(key);
	m_keysRepeat.insert(key);
}

void Input::UpdateKeyReleased(XVirtualKey key)
{
	m_keysReleased.insert(key);
	m_keysRepeat.erase(key);
}

void Input::UpdateAnalogState(const AnalogState& state)
{
	m_analogState = state;
}

void Input::UpdateAnalogState(AnalogState* state)
{
	// implicit copy constructor
	m_analogState = *state;
}

void Input::UpdateWheel(int32 wheelDelta)
{
	m_wheelDelta = wheelDelta;
}

void Input::UpdateCursorPosition(const glm::vec2& newCursorPosition)
{
	m_cursorRelativePosition = newCursorPosition - m_cursorPosition;
	m_cursorPosition = newCursorPosition;
}

void Input::UpdateDoubleClick()
{
	m_doubleClicked = true;
}

void Input::UpdateCursorDrag()
{
	m_cursorDragged = true;
}

bool Input::IsKeyPressed(XVirtualKey key) const
{
	return m_keysPressed.find(key) != m_keysPressed.end();
}

bool Input::IsKeyRepeat(XVirtualKey key) const
{
	return m_keysRepeat.find(key) != m_keysRepeat.end();
}

bool Input::IsKeyReleased(XVirtualKey key) const
{
	return m_keysReleased.find(key) != m_keysReleased.end();
}

void Input::ClearSoftState()
{
	m_wheelDelta = 0;
	m_cursorRelativePosition = { 0, 0 };
	m_doubleClicked = false;
	m_cursorDragged = false;

	m_keysReleased.clear();
	m_keysPressed.clear();
}
