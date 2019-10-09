#pragma once

#include <Xinput.h>
#include "core/enum/InputEnums.h"


inline bool MapLeftRightKeys(WPARAM& virtualKey, LPARAM lParam)
{
	const UINT scanCode = (lParam & 0x00ff0000) >> 16;
	const int32 extended = (lParam & 0x01000000) != 0;

	switch (virtualKey) {
		case VK_SHIFT: virtualKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX); break;
		case VK_CONTROL: virtualKey = extended ? VK_RCONTROL : VK_LCONTROL; break;
		case VK_MENU: virtualKey = extended ? VK_RMENU : VK_LMENU; break;
		default: return false;
	}

	return true;
}

inline WPARAM MapExtraMouseButtons(WPARAM wParam)
{
	switch (wParam) {
		case 0x0001: return VK_XBUTTON1;
		case 0x0002: return VK_XBUTTON2;
		default: return VK_XBUTTON1;
	}
}

inline XVirtualKey TranslateXInputVirtualKeys(WORD vkey)
{
	switch (vkey) {

		case VK_PAD_A: return XVirtualKey::GAMEPAD_A;
		case VK_PAD_B: return XVirtualKey::GAMEPAD_B;
		case VK_PAD_X: return XVirtualKey::GAMEPAD_X;
		case VK_PAD_Y: return XVirtualKey::GAMEPAD_Y;
		case VK_PAD_RSHOULDER: return XVirtualKey::GAMEPAD_RIGHT_SHOULDER;
		case VK_PAD_LSHOULDER: return XVirtualKey::GAMEPAD_LEFT_SHOULDER;
		case VK_PAD_LTRIGGER: return XVirtualKey::GAMEPAD_LEFT_TRIGGER;
		case VK_PAD_RTRIGGER: return XVirtualKey::GAMEPAD_RIGHT_TRIGGER;
		case VK_PAD_DPAD_UP: return XVirtualKey::GAMEPAD_DPAD_UP;
		case VK_PAD_DPAD_DOWN: return XVirtualKey::GAMEPAD_DPAD_DOWN;
		case VK_PAD_DPAD_LEFT: return XVirtualKey::GAMEPAD_DPAD_LEFT;
		case VK_PAD_DPAD_RIGHT: return XVirtualKey::GAMEPAD_DPAD_RIGHT;
		case VK_PAD_START: return XVirtualKey::GAMEPAD_MENU;
		case VK_PAD_BACK: return XVirtualKey::GAMEPAD_VIEW;
		case VK_PAD_LTHUMB_PRESS: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_BUTTON;
		case VK_PAD_RTHUMB_PRESS: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_BUTTON;
		case VK_PAD_LTHUMB_UP: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_UP;
		case VK_PAD_LTHUMB_UPLEFT: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_UPLEFT;
		case VK_PAD_LTHUMB_UPRIGHT: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_UPRIGHT;
		case VK_PAD_LTHUMB_DOWN: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_DOWN;
		case VK_PAD_LTHUMB_DOWNRIGHT: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_DOWNRIGHT;
		case VK_PAD_LTHUMB_DOWNLEFT: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_DOWNLEFT;
		case VK_PAD_LTHUMB_RIGHT: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_RIGHT;
		case VK_PAD_LTHUMB_LEFT: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_LEFT;
		case VK_PAD_RTHUMB_UP: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_UP;
		case VK_PAD_RTHUMB_UPLEFT: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_UPLEFT;
		case VK_PAD_RTHUMB_UPRIGHT: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_UPRIGHT;
		case VK_PAD_RTHUMB_DOWN: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_DOWN;
		case VK_PAD_RTHUMB_DOWNRIGHT: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_DOWNRIGHT;
		case VK_PAD_RTHUMB_DOWNLEFT: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_DOWNLEFT;
		case VK_PAD_RTHUMB_RIGHT: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_RIGHT;
		case VK_PAD_RTHUMB_LEFT: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_LEFT;
		default: return XVirtualKey::UNASSIGNED;
	}
}

inline XVirtualKey TranslateWin32VirtualKeys(WPARAM vkey)
{
	switch (vkey) {
		case '~': return XVirtualKey::TILDE;
		case 0x00: return XVirtualKey::UNASSIGNED;
		case VK_LBUTTON: return XVirtualKey::LBUTTON;
		case VK_RBUTTON: return XVirtualKey::RBUTTON;
		case VK_CANCEL: return XVirtualKey::CANCEL;
		case VK_MBUTTON: return XVirtualKey::MBUTTON;
		case VK_XBUTTON1: return XVirtualKey::XBUTTON01;
		case VK_XBUTTON2: return XVirtualKey::XBUTTON02;
		case 0x07: return XVirtualKey::RESERVED;
		case VK_BACK: return XVirtualKey::BACKSPACE;
		case VK_TAB: return XVirtualKey::TAB;
		case 0x0A:
		case 0x0B: return XVirtualKey::RESERVED;
		case VK_CLEAR: return XVirtualKey::CLEAR;
		case VK_RETURN: return XVirtualKey::ENTER;
		case 0x0E:
		case 0x0F: return XVirtualKey::UNASSIGNED;
		case VK_SHIFT: return XVirtualKey::SHIFT;
		case VK_CONTROL: return XVirtualKey::CTRL;
		case VK_MENU: return XVirtualKey::ALT;
		case VK_PAUSE: return XVirtualKey::PAUSE;
		case VK_CAPITAL: return XVirtualKey::CAPSLOCK;
		case 0x15: return XVirtualKey::HANGUL;
		case 0x16: return XVirtualKey::UNASSIGNED;
		case VK_JUNJA: return XVirtualKey::JUNJA;
		case VK_FINAL: return XVirtualKey::FINAL;
		case 0x19: return XVirtualKey::KANJI;
		case 0x1A: return XVirtualKey::UNASSIGNED;
		case VK_ESCAPE: return XVirtualKey::ESC;
		case VK_CONVERT: return XVirtualKey::CONVERT;
		case VK_NONCONVERT: return XVirtualKey::NONCONVERT;
		case VK_ACCEPT: return XVirtualKey::ACCEPT;
		case VK_MODECHANGE: return XVirtualKey::MODECHANGE;
		case VK_SPACE: return XVirtualKey::SPACEBAR;
		case VK_PRIOR: return XVirtualKey::PAGEUP;
		case VK_NEXT: return XVirtualKey::PAGEDOWN;
		case VK_END: return XVirtualKey::END;
		case VK_HOME: return XVirtualKey::HOME;
		case VK_LEFT: return XVirtualKey::LEFT;
		case VK_UP: return XVirtualKey::UP;
		case VK_RIGHT: return XVirtualKey::RIGHT;
		case VK_DOWN: return XVirtualKey::DOWN;
		case VK_SELECT: return XVirtualKey::SELECT;
		case VK_PRINT: return XVirtualKey::PRINT;
		case VK_EXECUTE: return XVirtualKey::EXECUTE;
		case VK_SNAPSHOT: return XVirtualKey::PRINTSCREEN;
		case VK_INSERT: return XVirtualKey::INSERT;
		case VK_DELETE: return XVirtualKey::DEL;
		case VK_HELP: return XVirtualKey::HELP;
		case 0x30: return XVirtualKey::K0;
		case 0x31: return XVirtualKey::K1;
		case 0x32: return XVirtualKey::K2;
		case 0x33: return XVirtualKey::K3;
		case 0x34: return XVirtualKey::K4;
		case 0x35: return XVirtualKey::K5;
		case 0x36: return XVirtualKey::K6;
		case 0x37: return XVirtualKey::K7;
		case 0x38: return XVirtualKey::K8;
		case 0x39: return XVirtualKey::K9;
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3E:
		case 0x3F:
		case 0x40: return XVirtualKey::UNASSIGNED;
		case 0x41: return XVirtualKey::A;
		case 0x42: return XVirtualKey::B;
		case 0x43: return XVirtualKey::C;
		case 0x44: return XVirtualKey::D;
		case 0x45: return XVirtualKey::E;
		case 0x46: return XVirtualKey::F;
		case 0x47: return XVirtualKey::G;
		case 0x48: return XVirtualKey::H;
		case 0x49: return XVirtualKey::I;
		case 0x4A: return XVirtualKey::J;
		case 0x4B: return XVirtualKey::K;
		case 0x4C: return XVirtualKey::L;
		case 0x4D: return XVirtualKey::M;
		case 0x4E: return XVirtualKey::N;
		case 0x4F: return XVirtualKey::O;
		case 0x50: return XVirtualKey::P;
		case 0x51: return XVirtualKey::Q;
		case 0x52: return XVirtualKey::R;
		case 0x53: return XVirtualKey::S;
		case 0x54: return XVirtualKey::T;
		case 0x55: return XVirtualKey::U;
		case 0x56: return XVirtualKey::V;
		case 0x57: return XVirtualKey::W;
		case 0x58: return XVirtualKey::X;
		case 0x59: return XVirtualKey::Y;
		case 0x5A: return XVirtualKey::Z;
		case VK_LWIN: return XVirtualKey::LWINDOWS;
		case VK_RWIN: return XVirtualKey::RWINDOWS;
		case VK_APPS: return XVirtualKey::APPS;
		case 0x5E: return XVirtualKey::RESERVED;
		case VK_SLEEP: return XVirtualKey::SLEEP;
		case VK_NUMPAD0: return XVirtualKey::NUM0;
		case VK_NUMPAD1: return XVirtualKey::NUM1;
		case VK_NUMPAD2: return XVirtualKey::NUM2;
		case VK_NUMPAD3: return XVirtualKey::NUM3;
		case VK_NUMPAD4: return XVirtualKey::NUM4;
		case VK_NUMPAD5: return XVirtualKey::NUM5;
		case VK_NUMPAD6: return XVirtualKey::NUM6;
		case VK_NUMPAD7: return XVirtualKey::NUM7;
		case VK_NUMPAD8: return XVirtualKey::NUM8;
		case VK_NUMPAD9: return XVirtualKey::NUM9;
		case VK_MULTIPLY: return XVirtualKey::MULTIPLY;
		case VK_ADD: return XVirtualKey::ADD;
		case VK_SEPARATOR: return XVirtualKey::SEPARATOR;
		case VK_SUBTRACT: return XVirtualKey::SUBTRACT;
		case VK_DECIMAL: return XVirtualKey::DECIMAL;
		case VK_DIVIDE: return XVirtualKey::DIVIDE;
		case VK_F1: return XVirtualKey::F1;
		case VK_F2: return XVirtualKey::F2;
		case VK_F3: return XVirtualKey::F3;
		case VK_F4: return XVirtualKey::F4;
		case VK_F5: return XVirtualKey::F5;
		case VK_F6: return XVirtualKey::F6;
		case VK_F7: return XVirtualKey::F7;
		case VK_F8: return XVirtualKey::F8;
		case VK_F9: return XVirtualKey::F9;
		case VK_F10: return XVirtualKey::F10;
		case VK_F11: return XVirtualKey::F11;
		case VK_F12: return XVirtualKey::F12;
		case VK_F13: return XVirtualKey::F13;
		case VK_F14: return XVirtualKey::F14;
		case VK_F15: return XVirtualKey::F15;
		case VK_F16: return XVirtualKey::F16;
		case VK_F17: return XVirtualKey::F17;
		case VK_F18: return XVirtualKey::F18;
		case VK_F19: return XVirtualKey::F19;
		case VK_F20: return XVirtualKey::F20;
		case VK_F21: return XVirtualKey::F21;
		case VK_F22: return XVirtualKey::F22;
		case VK_F23: return XVirtualKey::F23;
		case VK_F24: return XVirtualKey::F24;
		case VK_NAVIGATION_VIEW: return XVirtualKey::NAVIGATION_VIEW;
		case VK_NAVIGATION_MENU: return XVirtualKey::NAVIGATION_MENU;
		case VK_NAVIGATION_UP: return XVirtualKey::NAVIGATION_UP;
		case VK_NAVIGATION_DOWN: return XVirtualKey::NAVIGATION_DOWN;
		case VK_NAVIGATION_LEFT: return XVirtualKey::NAVIGATION_LEFT;
		case VK_NAVIGATION_RIGHT: return XVirtualKey::NAVIGATION_RIGHT;
		case VK_NAVIGATION_ACCEPT: return XVirtualKey::NAVIGATION_ACCEPT;
		case VK_NAVIGATION_CANCEL: return XVirtualKey::NAVIGATION_CANCEL;
		case VK_NUMLOCK: return XVirtualKey::NUMLOCK;
		case VK_SCROLL: return XVirtualKey::SCROLL;
		case 0x92: return XVirtualKey::OEM_FJ_JISHO;
		case VK_OEM_FJ_MASSHOU: return XVirtualKey::OEM_FJ_MASSHOU;
		case VK_OEM_FJ_TOUROKU: return XVirtualKey::OEM_FJ_TOUROKU;
		case VK_OEM_FJ_LOYA: return XVirtualKey::OEM_FJ_LOYA;
		case VK_OEM_FJ_ROYA: return XVirtualKey::OEM_FJ_ROYA;
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F: return XVirtualKey::UNASSIGNED;
		case VK_LSHIFT: return XVirtualKey::LSHIFT;
		case VK_RSHIFT: return XVirtualKey::RSHIFT;
		case VK_LCONTROL: return XVirtualKey::LCONTROL;
		case VK_RCONTROL: return XVirtualKey::RCONTROL;
		case VK_LMENU: return XVirtualKey::LALT;
		case VK_RMENU: return XVirtualKey::RALT;
		case VK_BROWSER_BACK: return XVirtualKey::BROWSER_BACK;
		case VK_BROWSER_FORWARD: return XVirtualKey::BROWSER_FORWARD;
		case VK_BROWSER_REFRESH: return XVirtualKey::BROWSER_REFRESH;
		case VK_BROWSER_STOP: return XVirtualKey::BROWSER_STOP;
		case VK_BROWSER_SEARCH: return XVirtualKey::BROWSER_SEARCH;
		case VK_BROWSER_FAVORITES: return XVirtualKey::BROWSER_FAVORITES;
		case VK_BROWSER_HOME: return XVirtualKey::BROWSER_HOME;
		case VK_VOLUME_MUTE: return XVirtualKey::VOLUME_MUTE;
		case VK_VOLUME_DOWN: return XVirtualKey::VOLUME_DOWN;
		case VK_VOLUME_UP: return XVirtualKey::VOLUME_UP;
		case VK_MEDIA_NEXT_TRACK: return XVirtualKey::MEDIA_NEXT_TRACK;
		case VK_MEDIA_PREV_TRACK: return XVirtualKey::MEDIA_PREV_TRACK;
		case VK_MEDIA_STOP: return XVirtualKey::MEDIA_STOP;
		case VK_MEDIA_PLAY_PAUSE: return XVirtualKey::MEDIA_PLAY_PAUSE;
		case VK_LAUNCH_MAIL: return XVirtualKey::LAUNCH_MAIL;
		case VK_LAUNCH_MEDIA_SELECT: return XVirtualKey::LAUNCH_MEDIA_SELECT;
		case VK_LAUNCH_APP1: return XVirtualKey::LAUNCH_APP1;
		case VK_LAUNCH_APP2: return XVirtualKey::LAUNCH_APP2;
		case 0xB8:
		case 0xB9: return XVirtualKey::RESERVED;
		case VK_OEM_1: return XVirtualKey::OEM_1;
		case VK_OEM_PLUS: return XVirtualKey::OEM_PLUS;
		case VK_OEM_COMMA: return XVirtualKey::OEM_COMMA;
		case VK_OEM_MINUS: return XVirtualKey::OEM_MINUS;
		case VK_OEM_PERIOD: return XVirtualKey::OEM_PERIOD;
		case VK_OEM_2: return XVirtualKey::OEM_2;
		case VK_OEM_3: return XVirtualKey::OEM_3;
		case 0xC1:
		case 0xC2: return XVirtualKey::RESERVED;
		case VK_GAMEPAD_A: return XVirtualKey::GAMEPAD_A;
		case VK_GAMEPAD_B: return XVirtualKey::GAMEPAD_B;
		case VK_GAMEPAD_X: return XVirtualKey::GAMEPAD_X;
		case VK_GAMEPAD_Y: return XVirtualKey::GAMEPAD_Y;
		case VK_GAMEPAD_RIGHT_SHOULDER: return XVirtualKey::GAMEPAD_RIGHT_SHOULDER;
		case VK_GAMEPAD_LEFT_SHOULDER: return XVirtualKey::GAMEPAD_LEFT_SHOULDER;
		case VK_GAMEPAD_LEFT_TRIGGER: return XVirtualKey::GAMEPAD_LEFT_TRIGGER;
		case VK_GAMEPAD_RIGHT_TRIGGER: return XVirtualKey::GAMEPAD_RIGHT_TRIGGER;
		case VK_GAMEPAD_DPAD_UP: return XVirtualKey::GAMEPAD_DPAD_UP;
		case VK_GAMEPAD_DPAD_DOWN: return XVirtualKey::GAMEPAD_DPAD_DOWN;
		case VK_GAMEPAD_DPAD_LEFT: return XVirtualKey::GAMEPAD_DPAD_LEFT;
		case VK_GAMEPAD_DPAD_RIGHT: return XVirtualKey::GAMEPAD_DPAD_RIGHT;
		case VK_GAMEPAD_MENU: return XVirtualKey::GAMEPAD_MENU;
		case VK_GAMEPAD_VIEW: return XVirtualKey::GAMEPAD_VIEW;
		case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_BUTTON;
		case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_BUTTON;
		case VK_GAMEPAD_LEFT_THUMBSTICK_UP: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_UP;
		case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_DOWN;
		case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_RIGHT;
		case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT: return XVirtualKey::GAMEPAD_LEFT_THUMBSTICK_LEFT;
		case VK_GAMEPAD_RIGHT_THUMBSTICK_UP: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_UP;
		case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_DOWN;
		case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_RIGHT;
		case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT: return XVirtualKey::GAMEPAD_RIGHT_THUMBSTICK_LEFT;
		case VK_OEM_4: return XVirtualKey::OEM_4;
		case VK_OEM_5: return XVirtualKey::OEM_5;
		case VK_OEM_6: return XVirtualKey::OEM_6;
		case VK_OEM_7: return XVirtualKey::OEM_7;
		case VK_OEM_8: return XVirtualKey::OEM_8;
		case 0xE0: return XVirtualKey::RESERVED;
		case VK_OEM_AX: return XVirtualKey::OEM_AX;
		case VK_OEM_102: return XVirtualKey::OEM_102;
		case VK_ICO_HELP: return XVirtualKey::ICO_HELP;
		case VK_ICO_00: return XVirtualKey::ICO_00;
		case VK_PROCESSKEY: return XVirtualKey::PROCESSKEY;
		case VK_ICO_CLEAR: return XVirtualKey::ICO_CLEAR;
		case VK_PACKET: return XVirtualKey::PACKET;
		case 0xE8: return XVirtualKey::UNASSIGNED;
		case VK_OEM_RESET: return XVirtualKey::OEM_RESET;
		case VK_OEM_JUMP: return XVirtualKey::OEM_JUMP;
		case VK_OEM_PA1: return XVirtualKey::OEM_PA1;
		case VK_OEM_PA2: return XVirtualKey::OEM_PA2;
		case VK_OEM_PA3: return XVirtualKey::OEM_PA3;
		case VK_OEM_WSCTRL: return XVirtualKey::OEM_WSCTRL;
		case VK_OEM_CUSEL: return XVirtualKey::OEM_CUSEL;
		case VK_OEM_ATTN: return XVirtualKey::OEM_ATTN;
		case VK_OEM_FINISH: return XVirtualKey::OEM_FINISH;
		case VK_OEM_COPY: return XVirtualKey::OEM_COPY;
		case VK_OEM_AUTO: return XVirtualKey::OEM_AUTO;
		case VK_OEM_ENLW: return XVirtualKey::OEM_ENLW;
		case VK_OEM_BACKTAB: return XVirtualKey::OEM_BACKTAB;
		case VK_ATTN: return XVirtualKey::ATTN;
		case VK_CRSEL: return XVirtualKey::CRSEL;
		case VK_EXSEL: return XVirtualKey::EXSEL;
		case VK_EREOF: return XVirtualKey::EREOF;
		case VK_PLAY: return XVirtualKey::PLAY;
		case VK_ZOOM: return XVirtualKey::ZOOM;
		case VK_NONAME: return XVirtualKey::NONAME;
		case VK_PA1: return XVirtualKey::PA1;
		case VK_OEM_CLEAR: return XVirtualKey::OEM_CLEAR;
		case 0xFF: return XVirtualKey::RESERVED;
		default: return XVirtualKey::UNASSIGNED;
	}
}
