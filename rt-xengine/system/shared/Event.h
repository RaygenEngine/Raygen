#ifndef EVENT_H
#define EVENT_H

#include "Types.h"

struct CTickEvent
{
	float timestamp;
};

struct CMouseWheelEvent
{
	int32 horizontal;
	int32 vertical;
};

struct CMouseMotionEvent
{
	int32 state; // if > 0 then a button is pressed -->  drag
	int32 posX;
	int32 posY;
	int32 xRel;        /**< The relative motion in the X direction */
	int32 yRel;        /**< The relative motion in the Y direction */
};

struct CMouseButtonReleasedEvent
{
	int32 posX;
	int32 posY;
	int32 button;
};

struct CMouseButtonPressedEvent
{
	int32 posX;
	int32 posY;
	int32 button;
	uint8 clickCount;
};

struct CKeyPressedEvent
{
	int32 key;
	uint8 repeat;
};

struct CKeyReleasedEvent
{
	int32 key;
};

struct CGameControllerButtonPressedEvent
{
	int32 button;
};

struct CGameControllerButtonReleasedEvent
{
	int32 button;
};

struct CGameControllerLeftStickMotionEvent
{
	// x: 0, y: 1
	uint32 axis;
	int16 value;
};

struct CGameControllerRightStickMotionEvent
{
	// x: 0, y: 1
	uint32 axis;
	int16 value;
};

struct CGameControllerLeftTriggerMotionEvent
{
	int16 value;
};

struct CGameControllerRightTriggerMotionEvent
{
	int16 value;
};

struct XPose
{
	// glm based alignment 
	struct
	{
		float x;
		float y;
		float z;
		float w;
	} orientation;

	struct
	{
		float x;
		float y;
		float z;
	} position;
};

struct COculusTrackingStateChangedEvent
{
	XPose head;
	XPose leftEye;
	XPose rightEye;
};

typedef enum
{
	// tick events
	ET_UPDATE = 0,
	ET_PRE_INPUT,
	ET_POST_INPUT,
	ET_PRE_RENDER,
	ET_POST_RENDER,

	// keyboard
	ET_KEY_PRESSED,
	ET_KEY_RELEASED,

	// mouse
	ET_MOUSE_PRESSED,
	ET_MOUSE_RELEASED,
	ET_MOUSE_MOTION,
	ET_MOUSE_WHEEL,

	// game controller(pad) 
	ET_GAME_CONTROLLER_BUTTON_PRESSED,
	ET_GAME_CONTROLLER_BUTTON_RELEASED,
	ET_GAME_CONTROLLER_LEFT_STICK_MOTION,
	ET_GAME_CONTROLLER_RIGHT_STICK_MOTION,
	ET_GAME_CONTROLLER_LEFT_TRIGGER_MOTION,
	ET_GAME_CONTROLLER_RIGHT_TRIGGER_MOTION,

	// oculus
	ET_OCULUS_TRACKING_STATE_CHANGED,
	ET_OCULUS_TIMEWARP_STATE_CHANGED,

	// invalid
	ET_INVALID
} EventType;

struct EngineEvent
{
	uint32 type;

	union
	{
		CTickEvent tickEvent;
		CMouseWheelEvent mouseWheelEvent;
		CMouseMotionEvent mouseMotionEvent;
		CMouseButtonReleasedEvent mouseButtonReleasedEvent;
		CMouseButtonPressedEvent mouseButtonPressedEvent;
		CKeyPressedEvent keyPressedEvent;
		CKeyReleasedEvent keyReleasedEvent;
		CGameControllerButtonPressedEvent gameControllerButtonPressedEvent;
		CGameControllerButtonReleasedEvent gameControllerButtonReleasedEvent;
		CGameControllerLeftStickMotionEvent gameControllerLeftStickMotionEvent;
		CGameControllerRightStickMotionEvent gameControllerRightStickMotionEvent;
		CGameControllerLeftTriggerMotionEvent gameControllerLeftTriggerMotionEvent;
		CGameControllerRightTriggerMotionEvent gameControllerRightTriggerMotionEvent;
		COculusTrackingStateChangedEvent oculusTrackingStateChangedEvent;
	};
};

#endif // EVENT_H
