#pragma once

#include "system/InputEnums.h"
#include "core/MathAux.h"

#include <unordered_set>

class Input {
public:
	struct Stick {
		float magnitude;
		glm::vec2 direction;

		float GetXAxisValue(float multiplyBy = 1.f) const { return direction.x * magnitude * multiplyBy; }
		float GetYAxisValue(float multiplyBy = 1.f) const { return direction.y * magnitude * multiplyBy; }
	};

	struct AnalogState {
		Stick ls; // left stick
		Stick rs; // right stick

		// normalized analog value
		float lt; // left trigger
		float rt; // right trigger
	};

private:
	std::unordered_set<Key> m_keysPressed;
	std::unordered_set<Key> m_keysRepeat;
	std::unordered_set<Key> m_keysReleased;

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

	void UpdateKeyPressed(Key key);
	void UpdateKeyReleased(Key key);

	void UpdateAnalogState(const AnalogState& state);
	void UpdateAnalogState(AnalogState* state);

	[[nodiscard]] bool IsKeyPressed(Key key) const;
	[[nodiscard]] bool IsKeyRepeat(Key key) const;
	[[nodiscard]] bool IsKeyReleased(Key key) const;

	[[nodiscard]] bool IsKeysCombination(Key key) const { return IsKeyRepeat(key); }

	template<typename... Args>
	[[nodiscard]] bool IsKeysCombination(Key key0, Args... keys) const
	{
		return IsKeysCombination(key0) && IsKeysCombination(keys...);
	}

	[[nodiscard]] bool IsAnyOfKeysPressed(Key key) const { return IsKeyPressed(key); }

	template<typename... Args>
	[[nodiscard]] bool IsAnyOfKeysPressed(Key key0, Args... keys) const
	{
		return IsAnyOfKeysPressed(key0) || IsAnyOfKeysPressed(keys...);
	}

	[[nodiscard]] bool IsAnyOfKeysRepeat(Key key) const { return IsKeyRepeat(key); }

	template<typename... Args>
	[[nodiscard]] bool IsAnyOfKeysRepeat(Key key0, Args... keys) const
	{
		return IsAnyOfKeysRepeat(key0) || IsAnyOfKeysRepeat(keys...);
	}

	[[nodiscard]] bool IsAnyOfKeysReleased(Key key) const { return IsKeyReleased(key); }

	template<typename... Args>
	[[nodiscard]] bool IsAnyOfKeysReleased(Key key0, Args... keys) const
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
	// multiples of 120
	[[nodiscard]] int32 GetWheelDelta() const { return m_wheelDelta; }

	[[nodiscard]] const AnalogState& GetAnalogState() const { return m_analogState; }
	[[nodiscard]] float GetLeftTriggerMagnitude() const { return m_analogState.lt; }
	[[nodiscard]] float GetRightTriggerMagnitude() const { return m_analogState.rt; }

	[[nodiscard]] float GetLeftThumbMagnitude() const { return m_analogState.ls.magnitude; }
	[[nodiscard]] float GetRightThumbMagnitude() const { return m_analogState.rs.magnitude; }

	[[nodiscard]] glm::vec2 GetLeftThumbDirection() const { return m_analogState.ls.direction; }
	[[nodiscard]] glm::vec2 GetRightThumbDirection() const { return m_analogState.rs.direction; }

	[[nodiscard]] bool IsLeftTriggerResting() const { return math::EpsilonEqualsZero(m_analogState.lt); }
	[[nodiscard]] bool IsRightTriggerResting() const { return math::EpsilonEqualsZero(m_analogState.rt); }

	[[nodiscard]] bool IsLeftThumbResting() const { return math::EpsilonEqualsZero(m_analogState.ls.magnitude); }
	[[nodiscard]] bool IsRightThumbResting() const { return math::EpsilonEqualsZero(m_analogState.rs.magnitude); }

	void ClearSoftState();
};
