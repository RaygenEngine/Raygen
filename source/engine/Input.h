#pragma once
#include "core/MathUtl.h"
#include "engine/InputEnums.h"

#include <bitset>
#include <unordered_set>

using KeyStates = std::bitset<static_cast<int32>(Key::_NUM)>;

// TODO:
// * Gamepad Support
// CHECK: Workaround mouse reporting outside of window
// CHECK: On window focus loss, KeyUp all the keys to avoid sticky keys
// * Implement reflection for Input Enum keys to support editor and Key Properties
// * Modifier support for key presses, useful for the editor - circumvented by event calls
// * Drop mouse move event for glfwGetMousePosition()

inline struct Input {
	struct Stick {
		float magnitude{ 0.0f };
		glm::vec2 direction;

		float GetXAxisValue(float multiplyBy = 1.f) const { return direction.x * magnitude * multiplyBy; }
		float GetYAxisValue(float multiplyBy = 1.f) const { return direction.y * magnitude * multiplyBy; }
		bool IsResting() const { return math::equalsZero(magnitude); }
	};

	struct GamepadState {
		Stick ls; // left stick
		Stick rs; // right stick

		// normalized analog value
		float lt{ 0.0f }; // left trigger
		float rt{ 0.0f }; // right trigger

		[[nodiscard]] bool IsLTResting() const { return math::equalsZero(lt); }
		[[nodiscard]] bool IsRTResting() const { return math::equalsZero(rt); }
	};


private:
	GamepadState gamepadState;
	KeyStates keyStates{};

	// Use getter utilities for these
	std::unordered_set<Key> keysJustPressed;
	std::unordered_set<Key> keysJustReleased;

	bool isMouseDragging{ false };

	glm::vec2 cursorPosition;
	glm::vec2 relativeCursorPosition;

	int32 scrollWheelDelta{ 0 };

	// Handles "double" keys propagation. (eg: Releasing LShift while holding down RShift)
	void ReleaseSpecialKey(Key released, Key special);

public:
	// Use only through platform code to update the state.
	void Z_UpdateKeyPressed(Key key, Key special);
	void Z_UpdateKeyReleased(Key key, Key special);
	void Z_UpdateMouseMove(glm::vec2 newCoords);
	void Z_UpdateScrollWheel(int32 newDelta);
	void Z_ClearFrameState();

	// Special function that handles probable focus lost. Called when we only release the key if it was already down.
	// Cases of this is ImGui consumes KeyRelease but we are not sure if we have updated our state.
	void Z_FocusLostKeyRelease(Key key, Key special);

	[[nodiscard]] bool IsMouseDragging() const noexcept { return isMouseDragging; }

	[[nodiscard]] glm::vec2 GetMouseDelta() const noexcept { return relativeCursorPosition; }

	// Pixels in window buffer coordinates.
	// LIMITATION: Actual mouse position will be incorrect if mouse is outside of the window
	[[nodiscard]] glm::vec2 GetMousePosition() const noexcept { return cursorPosition; }

	// Returns a possibly negative integer number of scroll steps.
	[[nodiscard]] int32 GetScrollDelta() const noexcept { return scrollWheelDelta; }

	[[nodiscard]] bool IsDown(Key key) const noexcept { return keyStates[static_cast<int32>(key)]; }
	[[nodiscard]] bool IsJustPressed(Key key) const noexcept { return keysJustPressed.count(key) > 0; }
	[[nodiscard]] bool IsJustReleased(Key key) const noexcept { return keysJustReleased.count(key) > 0; }

	[[nodiscard]] const KeyStates& GetKeyStates() const noexcept { return keyStates; }
	[[nodiscard]] const GamepadState& GetGamepadState() const noexcept { return gamepadState; }

	[[nodiscard]] bool AreKeysDown(Key key) const noexcept { return IsDown(key); }

	template<typename... Keys>
	[[nodiscard]] bool AreKeysDown(Key key, Keys... keys) const noexcept
	{
		return IsDown(key) && AreKeysDown(keys...);
	}


	// Pixels in game viewport coordinates from top left
	// uses g_viewportCoordinates
	[[nodiscard]] glm::vec2 GetMouseViewportPosition() const noexcept;
	[[nodiscard]] bool IsMouseInViewport() const noexcept;

	// Percentage from top left of the mouse position relative to the viewport
	[[nodiscard]] glm::vec2 GetMouseViewportUV() const noexcept;


	//
	// State modification functions
	//


	// Locks to main window
	// TODO: only used in editor drag, but current implementation disallows usage anywhere else. fix this
	void LockMouse();
	void UnlockMouse();
	void SetMouseLock(bool setLocked) { setLocked ? LockMouse() : UnlockMouse(); }

	// Relative to main window coordinates (GetMousePosition())
	void SetMousePos(glm::vec2 newPos);
} Input{};
