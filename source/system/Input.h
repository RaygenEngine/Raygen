#pragma once

#include <unordered_set>


class Input {
public:
	struct Thumb {
		float magnitude;
		glm::vec2 direction;
	};

	struct AnalogState {
		Thumb thumbL;
		Thumb thumbR;

		// normalized analog value
		float triggerL;
		float triggerR;
	};

private:
	std::unordered_set<XVirtualKey> m_keysPressed;
	std::unordered_set<XVirtualKey> m_keysRepeat;
	std::unordered_set<XVirtualKey> m_keysReleased;

	bool m_doubleClicked;
	bool m_cursorDragged;

	glm::vec2 m_cursorPosition;
	// rel to previous recorded m_cursorPosition
	glm::vec2 m_cursorRelativePosition;

	// multiples of 120
	int32 m_wheelDelta;

	AnalogState m_analogState{};

public:
	Input();

	void UpdateWheel(int32 wheelDelta);
	void UpdateCursorPosition(const glm::vec2& newCursorPosition);
	void UpdateDoubleClick();
	void UpdateCursorDrag();

	void UpdateKeyPressed(XVirtualKey key);
	void UpdateKeyReleased(XVirtualKey key);

	void UpdateAnalogState(const AnalogState& state);
	void UpdateAnalogState(AnalogState* state);

	bool IsKeyPressed(XVirtualKey key) const;
	bool IsKeyRepeat(XVirtualKey key) const;
	bool IsKeyReleased(XVirtualKey key) const;

	bool IsKeysCombination(XVirtualKey key) const { return IsKeyRepeat(key); }

	template<typename... Args>
	bool IsKeysCombination(XVirtualKey key0, Args... keys) const
	{
		return IsKeysCombination(key0) && IsKeysCombination(keys...);
	}

	bool IsAnyOfKeysPressed(XVirtualKey key) const { return IsKeyPressed(key); }

	template<typename... Args>
	bool IsAnyOfKeysPressed(XVirtualKey key0, Args... keys) const
	{
		return IsAnyOfKeysPressed(key0) || IsAnyOfKeysPressed(keys...);
	}

	bool IsAnyOfKeysRepeat(XVirtualKey key) const { return IsKeyRepeat(key); }

	template<typename... Args>
	bool IsAnyOfKeysRepeat(XVirtualKey key0, Args... keys) const
	{
		return IsAnyOfKeysRepeat(key0) || IsAnyOfKeysRepeat(keys...);
	}

	bool IsAnyOfKeysReleased(XVirtualKey key) const { return IsKeyReleased(key); }

	template<typename... Args>
	bool IsAnyOfKeysReleased(XVirtualKey key0, Args... keys) const
	{
		return IsAnyOfKeysReleased(key0) || IsAnyOfKeysReleased(keys...);
	}

	bool IsDoubleClicked() const { return m_cursorDragged; }
	bool IsCursorDragged() const { return m_cursorDragged; }
	// with regards to the previous recorded
	glm::vec2 GetCursorRelativePosition() const { return m_cursorRelativePosition; }
	glm::vec2 GetCursorPosition() const { return m_cursorPosition; }
	int32 GetWheelDelta() const { return m_wheelDelta; }

	const AnalogState& GetAnalogState() const { return m_analogState; }
	float GetLeftTriggerMagnitude() const { return m_analogState.triggerL; }
	float GetRightTriggerMagnitude() const { return m_analogState.triggerR; }

	float GetLeftThumbMagnitude() const { return m_analogState.thumbL.magnitude; }
	float GetRightThumbMagnitude() const { return m_analogState.thumbR.magnitude; }

	glm::vec2 GetLeftThumbDirection() const { return m_analogState.thumbL.direction; }
	glm::vec2 GetRightThumbDirection() const { return m_analogState.thumbR.direction; }

	bool IsLeftTriggerResting() const { return utl::EqualsZero(m_analogState.triggerL); }
	bool IsRightTriggerResting() const { return utl::EqualsZero(m_analogState.triggerR); }

	bool IsLeftThumbResting() const { return utl::EqualsZero(m_analogState.thumbL.magnitude); }
	bool IsRightThumbResting() const { return utl::EqualsZero(m_analogState.thumbR.magnitude); }

	bool IsLeftTriggerMoving() const { return !utl::EqualsZero(m_analogState.triggerL); }
	bool IsRightTriggerMoving() const { return !utl::EqualsZero(m_analogState.triggerR); }

	bool IsLeftThumbMoving() const { return !utl::EqualsZero(m_analogState.thumbL.magnitude); }
	bool IsRightThumbMoving() const { return !utl::EqualsZero(m_analogState.thumbR.magnitude); }

	void ClearSoftState();
};
