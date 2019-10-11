#pragma once

#include "system/InputEnums.h"

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

	bool m_doubleClicked{ false };
	bool m_cursorDragged{ false };

	glm::vec2 m_cursorPosition{};
	// rel to previous recorded m_cursorPosition
	glm::vec2 m_cursorRelativePosition{};

	// multiples of 120
	int32 m_wheelDelta{ 0 };

	AnalogState m_analogState{};

public:
	void UpdateWheel(int32 wheelDelta);
	void UpdateCursorPosition(const glm::vec2& newCursorPosition);
	void UpdateDoubleClick();
	void UpdateCursorDrag();

	void UpdateKeyPressed(XVirtualKey key);
	void UpdateKeyReleased(XVirtualKey key);

	void UpdateAnalogState(const AnalogState& state);
	void UpdateAnalogState(AnalogState* state);

	[[nodiscard]] bool IsKeyPressed(XVirtualKey key) const;
	[[nodiscard]] bool IsKeyRepeat(XVirtualKey key) const;
	[[nodiscard]] bool IsKeyReleased(XVirtualKey key) const;

	[[nodiscard]] bool IsKeysCombination(XVirtualKey key) const { return IsKeyRepeat(key); }

	template<typename... Args>
	[[nodiscard]] bool IsKeysCombination(XVirtualKey key0, Args... keys) const
	{
		return IsKeysCombination(key0) && IsKeysCombination(keys...);
	}

	[[nodiscard]] bool IsAnyOfKeysPressed(XVirtualKey key) const { return IsKeyPressed(key); }

	template<typename... Args>
	[[nodiscard]] bool IsAnyOfKeysPressed(XVirtualKey key0, Args... keys) const
	{
		return IsAnyOfKeysPressed(key0) || IsAnyOfKeysPressed(keys...);
	}

	[[nodiscard]] bool IsAnyOfKeysRepeat(XVirtualKey key) const { return IsKeyRepeat(key); }

	template<typename... Args>
	[[nodiscard]] bool IsAnyOfKeysRepeat(XVirtualKey key0, Args... keys) const
	{
		return IsAnyOfKeysRepeat(key0) || IsAnyOfKeysRepeat(keys...);
	}

	[[nodiscard]] bool IsAnyOfKeysReleased(XVirtualKey key) const { return IsKeyReleased(key); }

	template<typename... Args>
	[[nodiscard]] bool IsAnyOfKeysReleased(XVirtualKey key0, Args... keys) const
	{
		return IsAnyOfKeysReleased(key0) || IsAnyOfKeysReleased(keys...);
	}

	[[nodiscard]] bool IsDoubleClicked() const { return m_cursorDragged; }
	[[nodiscard]] bool IsCursorDragged() const { return m_cursorDragged; }
	[[nodiscard]] // with regards to the previous recorded
	[[nodiscard]] glm::vec2
		GetCursorRelativePosition() const
	{
		return m_cursorRelativePosition;
	}
	[[nodiscard]] glm::vec2 GetCursorPosition() const { return m_cursorPosition; }
	[[nodiscard]] int32 GetWheelDelta() const { return m_wheelDelta; }

	[[nodiscard]] const AnalogState& GetAnalogState() const { return m_analogState; }
	[[nodiscard]] float GetLeftTriggerMagnitude() const { return m_analogState.triggerL; }
	[[nodiscard]] float GetRightTriggerMagnitude() const { return m_analogState.triggerR; }

	[[nodiscard]] float GetLeftThumbMagnitude() const { return m_analogState.thumbL.magnitude; }
	[[nodiscard]] float GetRightThumbMagnitude() const { return m_analogState.thumbR.magnitude; }

	[[nodiscard]] glm::vec2 GetLeftThumbDirection() const { return m_analogState.thumbL.direction; }
	[[nodiscard]] glm::vec2 GetRightThumbDirection() const { return m_analogState.thumbR.direction; }

	[[nodiscard]] bool IsLeftTriggerResting() const { return math::EpsilonEqualsZero(m_analogState.triggerL); }
	[[nodiscard]] bool IsRightTriggerResting() const { return math::EpsilonEqualsZero(m_analogState.triggerR); }

	[[nodiscard]] bool IsLeftThumbResting() const { return math::EpsilonEqualsZero(m_analogState.thumbL.magnitude); }
	[[nodiscard]] bool IsRightThumbResting() const { return math::EpsilonEqualsZero(m_analogState.thumbR.magnitude); }

	void ClearSoftState();
};
