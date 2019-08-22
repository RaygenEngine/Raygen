#ifndef TRANSLATEWIN32VIRTUALKEYS_H
#define TRANSLATEWIN32VIRTUALKEYS_H

#include <system/shared/InputEnums.h>

inline bool MapLeftRightKeys(WPARAM& virtualKey, LPARAM lParam)
{
	const UINT scanCode = (lParam & 0x00ff0000) >> 16;
	const int32 extended = (lParam & 0x01000000) != 0;

	switch (virtualKey)
	{
	case VK_SHIFT:
		virtualKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
		break;
	case VK_CONTROL:
		virtualKey = extended ? VK_RCONTROL : VK_LCONTROL;
		break;
	case VK_MENU:
		virtualKey = extended ? VK_RMENU : VK_LMENU;
		break;
	default:
		return false;
	}

	return true;
}

inline WPARAM MapExtraMouseButtons(WPARAM wParam)
{
	switch (wParam)
	{
	case 0x0001: return VK_XBUTTON1;
	case 0x0002: return VK_XBUTTON2;
	default:	 return VK_XBUTTON1;
	}
}

#ifdef XINPUT_CONTROLLER_ENABLED

#include <Xinput.h>
inline XVirtualKey TranslateXInputVirtualKeys(WORD vkey)
{
	switch (vkey)
	{

	case VK_PAD_A:					return XVK_GAMEPAD_A;
	case VK_PAD_B:					return XVK_GAMEPAD_B;
	case VK_PAD_X:					return XVK_GAMEPAD_X;
	case VK_PAD_Y:					return XVK_GAMEPAD_Y;
	case VK_PAD_RSHOULDER:			return XVK_GAMEPAD_RIGHT_SHOULDER;
	case VK_PAD_LSHOULDER:			return XVK_GAMEPAD_LEFT_SHOULDER;
	case VK_PAD_LTRIGGER:			return XVK_GAMEPAD_LEFT_TRIGGER;
	case VK_PAD_RTRIGGER:			return XVK_GAMEPAD_RIGHT_TRIGGER;
	case VK_PAD_DPAD_UP:			return XVK_GAMEPAD_DPAD_UP;
	case VK_PAD_DPAD_DOWN:			return XVK_GAMEPAD_DPAD_DOWN;
	case VK_PAD_DPAD_LEFT:			return XVK_GAMEPAD_DPAD_LEFT;
	case VK_PAD_DPAD_RIGHT:			return XVK_GAMEPAD_DPAD_RIGHT;
	case VK_PAD_START:				return XVK_GAMEPAD_MENU;
	case VK_PAD_BACK:				return XVK_GAMEPAD_VIEW;
	case VK_PAD_LTHUMB_PRESS:		return XVK_GAMEPAD_LEFT_THUMBSTICK_BUTTON;
	case VK_PAD_RTHUMB_PRESS:		return XVK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON;
	case VK_PAD_LTHUMB_UP:			return XVK_GAMEPAD_LEFT_THUMBSTICK_UP;
	case VK_PAD_LTHUMB_UPLEFT:		return XVK_GAMEPAD_LEFT_THUMBSTICK_UPLEFT;
	case VK_PAD_LTHUMB_UPRIGHT:		return XVK_GAMEPAD_LEFT_THUMBSTICK_UPRIGHT;  
	case VK_PAD_LTHUMB_DOWN:		return XVK_GAMEPAD_LEFT_THUMBSTICK_DOWN;
	case VK_PAD_LTHUMB_DOWNRIGHT:	return XVK_GAMEPAD_LEFT_THUMBSTICK_DOWNRIGHT;
	case VK_PAD_LTHUMB_DOWNLEFT:	return XVK_GAMEPAD_LEFT_THUMBSTICK_DOWNLEFT;
	case VK_PAD_LTHUMB_RIGHT:		return XVK_GAMEPAD_LEFT_THUMBSTICK_RIGHT;
	case VK_PAD_LTHUMB_LEFT:		return XVK_GAMEPAD_LEFT_THUMBSTICK_LEFT;
	case VK_PAD_RTHUMB_UP:			return XVK_GAMEPAD_RIGHT_THUMBSTICK_UP;
	case VK_PAD_RTHUMB_UPLEFT:		return XVK_GAMEPAD_RIGHT_THUMBSTICK_UPLEFT;
	case VK_PAD_RTHUMB_UPRIGHT:		return XVK_GAMEPAD_RIGHT_THUMBSTICK_UPRIGHT;
	case VK_PAD_RTHUMB_DOWN:		return XVK_GAMEPAD_RIGHT_THUMBSTICK_DOWN;
	case VK_PAD_RTHUMB_DOWNRIGHT:	return XVK_GAMEPAD_RIGHT_THUMBSTICK_DOWNRIGHT;
	case VK_PAD_RTHUMB_DOWNLEFT:	return XVK_GAMEPAD_RIGHT_THUMBSTICK_DOWNLEFT;
	case VK_PAD_RTHUMB_RIGHT:		return XVK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT;
	case VK_PAD_RTHUMB_LEFT:		return XVK_GAMEPAD_RIGHT_THUMBSTICK_LEFT;
	default:						return XVK_UNASSIGNED;
	}
}
#endif

inline XVirtualKey TranslateWin32VirtualKeys(WPARAM vkey)
{
	switch (vkey)
	{
	case 0x00:									return XVK_UNASSIGNED;
	case VK_LBUTTON:							return XVK_LBUTTON;
	case VK_RBUTTON:							return XVK_RBUTTON;
	case VK_CANCEL:								return XVK_CANCEL;
	case VK_MBUTTON:							return XVK_MBUTTON;
	case VK_XBUTTON1:							return XVK_XBUTTON1;
	case VK_XBUTTON2:							return XVK_XBUTTON2;
	case 0x07:									return XVK_RESERVED;
	case VK_BACK:								return XVK_BACKSPACE;
	case VK_TAB:								return XVK_TAB;
	case 0x0A:
	case 0x0B:									return XVK_RESERVED;
	case VK_CLEAR:								return XVK_CLEAR;
	case VK_RETURN:								return XVK_ENTER;
	case 0x0E:
	case 0x0F:									return XVK_UNASSIGNED;
	case VK_SHIFT:								return XVK_SHIFT;
	case VK_CONTROL:							return XVK_CTRL;
	case VK_MENU:								return XVK_ALT;
	case VK_PAUSE:								return XVK_PAUSE;
	case VK_CAPITAL:							return XVK_CAPSLOCK;
	case 0x15:									return XVK_HANGUL;
	case 0x16:									return XVK_UNASSIGNED;
	case VK_JUNJA:								return XVK_JUNJA;
	case VK_FINAL:								return XVK_FINAL;
	case 0x19:									return XVK_KANJI;
	case 0x1A:									return XVK_UNASSIGNED;
	case VK_ESCAPE:								return XVK_ESC;
	case VK_CONVERT:							return XVK_CONVERT;
	case VK_NONCONVERT:							return XVK_NONCONVERT;
	case VK_ACCEPT:								return XVK_ACCEPT;
	case VK_MODECHANGE:							return XVK_MODECHANGE;
	case VK_SPACE:								return XVK_SPACEBAR;
	case VK_PRIOR:								return XVK_PAGEUP;
	case VK_NEXT:								return XVK_PAGEDOWN;
	case VK_END:								return XVK_END;
	case VK_HOME:								return XVK_HOME;
	case VK_LEFT:								return XVK_LEFT;
	case VK_UP:									return XVK_UP;
	case VK_RIGHT:								return XVK_RIGHT;
	case VK_DOWN:								return XVK_DOWN;
	case VK_SELECT:								return XVK_SELECT;
	case VK_PRINT:								return XVK_PRINT;
	case VK_EXECUTE:							return XVK_EXECUTE;
	case VK_SNAPSHOT:							return XVK_PRINTSCREEN;
	case VK_INSERT:								return XVK_INSERT;
	case VK_DELETE:								return XVK_DELETE;
	case VK_HELP:								return XVK_HELP;
	case 0x30:									return XVK_0;
	case 0x31:									return XVK_1;
	case 0x32:									return XVK_2;
	case 0x33:									return XVK_3;
	case 0x34:									return XVK_4;
	case 0x35:									return XVK_5;
	case 0x36:									return XVK_6;
	case 0x37:									return XVK_7;
	case 0x38:									return XVK_8;
	case 0x39:									return XVK_9;
	case 0x3A:
	case 0x3B:
	case 0x3C:
	case 0x3D:
	case 0x3E:
	case 0x3F:
	case 0x40:									return XVK_UNASSIGNED;
	case 0x41:									return XVK_A;
	case 0x42:									return XVK_B;
	case 0x43:									return XVK_C;
	case 0x44:									return XVK_D;
	case 0x45:									return XVK_E;
	case 0x46:									return XVK_F;
	case 0x47:									return XVK_G;
	case 0x48:									return XVK_H;
	case 0x49:									return XVK_I;
	case 0x4A:									return XVK_J;
	case 0x4B:									return XVK_K;
	case 0x4C:									return XVK_L;
	case 0x4D:									return XVK_M;
	case 0x4E:									return XVK_N;
	case 0x4F:									return XVK_O;
	case 0x50:									return XVK_P;
	case 0x51:									return XVK_Q;
	case 0x52:									return XVK_R;
	case 0x53:									return XVK_S;
	case 0x54:									return XVK_T;
	case 0x55:									return XVK_U;
	case 0x56:									return XVK_V;
	case 0x57:									return XVK_W;
	case 0x58:									return XVK_X;
	case 0x59:									return XVK_Y;
	case 0x5A:									return XVK_Z;
	case VK_LWIN:								return XVK_LWINDOWS;
	case VK_RWIN:								return XVK_RWINDOWS;
	case VK_APPS:								return XVK_APPS;
	case 0x5E:									return XVK_RESERVED;
	case VK_SLEEP:								return XVK_SLEEP;
	case VK_NUMPAD0:							return XVK_NUM0;
	case VK_NUMPAD1:							return XVK_NUM1;
	case VK_NUMPAD2:							return XVK_NUM2;
	case VK_NUMPAD3:							return XVK_NUM3;
	case VK_NUMPAD4:							return XVK_NUM4;
	case VK_NUMPAD5:							return XVK_NUM5;
	case VK_NUMPAD6:							return XVK_NUM6;
	case VK_NUMPAD7:							return XVK_NUM7;
	case VK_NUMPAD8:							return XVK_NUM8;
	case VK_NUMPAD9:							return XVK_NUM9;
	case VK_MULTIPLY:							return XVK_MULTIPLY;
	case VK_ADD:								return XVK_ADD;
	case VK_SEPARATOR:							return XVK_SEPARATOR;
	case VK_SUBTRACT:							return XVK_SUBTRACT;
	case VK_DECIMAL:							return XVK_DECIMAL;
	case VK_DIVIDE:								return XVK_DIVIDE;
	case VK_F1:									return XVK_F1;
	case VK_F2:									return XVK_F2;
	case VK_F3:									return XVK_F3;
	case VK_F4:									return XVK_F4;
	case VK_F5:									return XVK_F5;
	case VK_F6:									return XVK_F6;
	case VK_F7:									return XVK_F7;
	case VK_F8:									return XVK_F8;
	case VK_F9:									return XVK_F9;
	case VK_F10:								return XVK_F10;
	case VK_F11:								return XVK_F11;
	case VK_F12:								return XVK_F12;
	case VK_F13:								return XVK_F13;
	case VK_F14:								return XVK_F14;
	case VK_F15:								return XVK_F15;
	case VK_F16:								return XVK_F16;
	case VK_F17:								return XVK_F17;
	case VK_F18:								return XVK_F18;
	case VK_F19:								return XVK_F19;
	case VK_F20:								return XVK_F20;
	case VK_F21:								return XVK_F21;
	case VK_F22:								return XVK_F22;
	case VK_F23:								return XVK_F23;
	case VK_F24:								return XVK_F24;
	case VK_NAVIGATION_VIEW:					return XVK_NAVIGATION_VIEW;
	case VK_NAVIGATION_MENU:					return XVK_NAVIGATION_MENU;
	case VK_NAVIGATION_UP:						return XVK_NAVIGATION_UP;
	case VK_NAVIGATION_DOWN:					return XVK_NAVIGATION_DOWN;
	case VK_NAVIGATION_LEFT:					return XVK_NAVIGATION_LEFT;
	case VK_NAVIGATION_RIGHT:					return XVK_NAVIGATION_RIGHT;
	case VK_NAVIGATION_ACCEPT:					return XVK_NAVIGATION_ACCEPT;
	case VK_NAVIGATION_CANCEL:					return XVK_NAVIGATION_CANCEL;
	case VK_NUMLOCK:							return XVK_NUMLOCK;
	case VK_SCROLL:								return XVK_SCROLL;
	case 0x92:									return XVK_OEM_FJ_JISHO;
	case VK_OEM_FJ_MASSHOU:						return XVK_OEM_FJ_MASSHOU;
	case VK_OEM_FJ_TOUROKU:						return XVK_OEM_FJ_TOUROKU;
	case VK_OEM_FJ_LOYA:						return XVK_OEM_FJ_LOYA;
	case VK_OEM_FJ_ROYA:						return XVK_OEM_FJ_ROYA;
	case 0x97:
	case 0x98:
	case 0x99:
	case 0x9A:
	case 0x9B:
	case 0x9C:
	case 0x9D:
	case 0x9E:
	case 0x9F:									return XVK_UNASSIGNED;
	case VK_LSHIFT:								return XVK_LSHIFT;
	case VK_RSHIFT:								return XVK_RSHIFT;
	case VK_LCONTROL:							return XVK_LCONTROL;
	case VK_RCONTROL:							return XVK_RCONTROL;
	case VK_LMENU:								return XVK_LALT;
	case VK_RMENU:								return XVK_RALT;
	case VK_BROWSER_BACK:						return XVK_BROWSER_BACK;
	case VK_BROWSER_FORWARD:					return XVK_BROWSER_FORWARD;
	case VK_BROWSER_REFRESH:					return XVK_BROWSER_REFRESH;
	case VK_BROWSER_STOP:						return XVK_BROWSER_STOP;
	case VK_BROWSER_SEARCH:						return XVK_BROWSER_SEARCH;
	case VK_BROWSER_FAVORITES:					return XVK_BROWSER_FAVORITES;
	case VK_BROWSER_HOME:						return XVK_BROWSER_HOME;
	case VK_VOLUME_MUTE:						return XVK_VOLUME_MUTE;
	case VK_VOLUME_DOWN:						return XVK_VOLUME_DOWN;
	case VK_VOLUME_UP:							return XVK_VOLUME_UP;
	case VK_MEDIA_NEXT_TRACK:					return XVK_MEDIA_NEXT_TRACK;
	case VK_MEDIA_PREV_TRACK:					return XVK_MEDIA_PREV_TRACK;
	case VK_MEDIA_STOP:							return XVK_MEDIA_STOP;
	case VK_MEDIA_PLAY_PAUSE:					return XVK_MEDIA_PLAY_PAUSE;
	case VK_LAUNCH_MAIL:						return XVK_LAUNCH_MAIL;
	case VK_LAUNCH_MEDIA_SELECT:				return XVK_LAUNCH_MEDIA_SELECT;
	case VK_LAUNCH_APP1:						return XVK_LAUNCH_APP1;
	case VK_LAUNCH_APP2:						return XVK_LAUNCH_APP2;
	case 0xB8:
	case 0xB9:									return XVK_RESERVED;
	case VK_OEM_1:								return XVK_OEM_1;
	case VK_OEM_PLUS:							return XVK_OEM_PLUS;
	case VK_OEM_COMMA:							return XVK_OEM_COMMA;
	case VK_OEM_MINUS:							return XVK_OEM_MINUS;
	case VK_OEM_PERIOD:							return XVK_OEM_PERIOD;
	case VK_OEM_2:								return XVK_OEM_2;
	case VK_OEM_3:								return XVK_OEM_3;
	case 0xC1:
	case 0xC2:									return XVK_RESERVED;
	case VK_GAMEPAD_A:							return XVK_GAMEPAD_A;
	case VK_GAMEPAD_B:							return XVK_GAMEPAD_B;
	case VK_GAMEPAD_X:							return XVK_GAMEPAD_X;
	case VK_GAMEPAD_Y:							return XVK_GAMEPAD_Y;
	case VK_GAMEPAD_RIGHT_SHOULDER:				return XVK_GAMEPAD_RIGHT_SHOULDER;
	case VK_GAMEPAD_LEFT_SHOULDER:				return XVK_GAMEPAD_LEFT_SHOULDER;
	case VK_GAMEPAD_LEFT_TRIGGER:				return XVK_GAMEPAD_LEFT_TRIGGER;
	case VK_GAMEPAD_RIGHT_TRIGGER:				return XVK_GAMEPAD_RIGHT_TRIGGER;
	case VK_GAMEPAD_DPAD_UP:					return XVK_GAMEPAD_DPAD_UP;
	case VK_GAMEPAD_DPAD_DOWN:					return XVK_GAMEPAD_DPAD_DOWN;
	case VK_GAMEPAD_DPAD_LEFT:					return XVK_GAMEPAD_DPAD_LEFT;
	case VK_GAMEPAD_DPAD_RIGHT:					return XVK_GAMEPAD_DPAD_RIGHT;
	case VK_GAMEPAD_MENU:						return XVK_GAMEPAD_MENU;
	case VK_GAMEPAD_VIEW:						return XVK_GAMEPAD_VIEW;
	case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON:		return XVK_GAMEPAD_LEFT_THUMBSTICK_BUTTON;
	case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON:	return XVK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON;
	case VK_GAMEPAD_LEFT_THUMBSTICK_UP:			return XVK_GAMEPAD_LEFT_THUMBSTICK_UP;
	case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN:		return XVK_GAMEPAD_LEFT_THUMBSTICK_DOWN;
	case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT:		return XVK_GAMEPAD_LEFT_THUMBSTICK_RIGHT;
	case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT:		return XVK_GAMEPAD_LEFT_THUMBSTICK_LEFT;
	case VK_GAMEPAD_RIGHT_THUMBSTICK_UP:		return XVK_GAMEPAD_RIGHT_THUMBSTICK_UP;
	case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN:		return XVK_GAMEPAD_RIGHT_THUMBSTICK_DOWN;
	case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT:		return XVK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT;
	case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT:		return XVK_GAMEPAD_RIGHT_THUMBSTICK_LEFT;
	case VK_OEM_4:								return XVK_OEM_4;
	case VK_OEM_5:								return XVK_OEM_5;
	case VK_OEM_6:								return XVK_OEM_6;
	case VK_OEM_7:								return XVK_OEM_7;
	case VK_OEM_8:								return XVK_OEM_8;
	case 0xE0:									return XVK_RESERVED;
	case VK_OEM_AX:								return XVK_OEM_AX;
	case VK_OEM_102:							return XVK_OEM_102;
	case VK_ICO_HELP:							return XVK_ICO_HELP;
	case VK_ICO_00:								return XVK_ICO_00;
	case VK_PROCESSKEY:							return XVK_PROCESSKEY;
	case VK_ICO_CLEAR:							return XVK_ICO_CLEAR;
	case VK_PACKET:								return XVK_PACKET;
	case 0xE8:									return XVK_UNASSIGNED;
	case VK_OEM_RESET:							return ΧVK_OEM_RESET;
	case VK_OEM_JUMP:							return ΧVK_OEM_JUMP;
	case VK_OEM_PA1:							return ΧVK_OEM_PA1;
	case VK_OEM_PA2:							return ΧVK_OEM_PA2;
	case VK_OEM_PA3:							return ΧVK_OEM_PA3;
	case VK_OEM_WSCTRL:							return ΧVK_OEM_WSCTRL;
	case VK_OEM_CUSEL:							return ΧVK_OEM_CUSEL;
	case VK_OEM_ATTN:							return ΧVK_OEM_ATTN;
	case VK_OEM_FINISH:							return ΧVK_OEM_FINISH;
	case VK_OEM_COPY:							return ΧVK_OEM_COPY;
	case VK_OEM_AUTO:							return ΧVK_OEM_AUTO;
	case VK_OEM_ENLW:							return ΧVK_OEM_ENLW;
	case VK_OEM_BACKTAB:						return ΧVK_OEM_BACKTAB;
	case VK_ATTN:								return ΧVK_ATTN;
	case VK_CRSEL:								return ΧVK_CRSEL;
	case VK_EXSEL:								return ΧVK_EXSEL;
	case VK_EREOF:								return ΧVK_EREOF;
	case VK_PLAY:								return ΧVK_PLAY;
	case VK_ZOOM:								return ΧVK_ZOOM;
	case VK_NONAME:								return ΧVK_NONAME;
	case VK_PA1:								return ΧVK_PA1;
	case VK_OEM_CLEAR:							return ΧVK_OEM_CLEAR;
	case 0xFF:									return XVK_RESERVED;
	default:									return XVK_UNASSIGNED;
	}
}

#endif // TRANSLATEWIN32VIRTUALKEYS_H
