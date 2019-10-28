#pragma once

#include "system/InputEnums.h"

#include <Xinput.h>

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

inline Key TranslateXInputVirtualKeys(WORD vkey)
{
	switch (vkey) {

		case VK_PAD_A: return Key::GAMEPAD_A;
		case VK_PAD_B: return Key::GAMEPAD_B;
		case VK_PAD_X: return Key::GAMEPAD_X;
		case VK_PAD_Y: return Key::GAMEPAD_Y;
		case VK_PAD_RSHOULDER: return Key::GAMEPAD_RIGHT_SHOULDER;
		case VK_PAD_LSHOULDER: return Key::GAMEPAD_LEFT_SHOULDER;
		case VK_PAD_LTRIGGER: return Key::GAMEPAD_LEFT_TRIGGER;
		case VK_PAD_RTRIGGER: return Key::GAMEPAD_RIGHT_TRIGGER;
		case VK_PAD_DPAD_UP: return Key::GAMEPAD_DPAD_UP;
		case VK_PAD_DPAD_DOWN: return Key::GAMEPAD_DPAD_DOWN;
		case VK_PAD_DPAD_LEFT: return Key::GAMEPAD_DPAD_LEFT;
		case VK_PAD_DPAD_RIGHT: return Key::GAMEPAD_DPAD_RIGHT;
		case VK_PAD_START: return Key::GAMEPAD_MENU;
		case VK_PAD_BACK: return Key::GAMEPAD_VIEW;
		case VK_PAD_LTHUMB_PRESS: return Key::GAMEPAD_LEFT_THUMBSTICK_BUTTON;
		case VK_PAD_RTHUMB_PRESS: return Key::GAMEPAD_RIGHT_THUMBSTICK_BUTTON;
		case VK_PAD_LTHUMB_UP: return Key::GAMEPAD_LEFT_THUMBSTICK_UP;
		case VK_PAD_LTHUMB_UPLEFT: return Key::GAMEPAD_LEFT_THUMBSTICK_UPLEFT;
		case VK_PAD_LTHUMB_UPRIGHT: return Key::GAMEPAD_LEFT_THUMBSTICK_UPRIGHT;
		case VK_PAD_LTHUMB_DOWN: return Key::GAMEPAD_LEFT_THUMBSTICK_DOWN;
		case VK_PAD_LTHUMB_DOWNRIGHT: return Key::GAMEPAD_LEFT_THUMBSTICK_DOWNRIGHT;
		case VK_PAD_LTHUMB_DOWNLEFT: return Key::GAMEPAD_LEFT_THUMBSTICK_DOWNLEFT;
		case VK_PAD_LTHUMB_RIGHT: return Key::GAMEPAD_LEFT_THUMBSTICK_RIGHT;
		case VK_PAD_LTHUMB_LEFT: return Key::GAMEPAD_LEFT_THUMBSTICK_LEFT;
		case VK_PAD_RTHUMB_UP: return Key::GAMEPAD_RIGHT_THUMBSTICK_UP;
		case VK_PAD_RTHUMB_UPLEFT: return Key::GAMEPAD_RIGHT_THUMBSTICK_UPLEFT;
		case VK_PAD_RTHUMB_UPRIGHT: return Key::GAMEPAD_RIGHT_THUMBSTICK_UPRIGHT;
		case VK_PAD_RTHUMB_DOWN: return Key::GAMEPAD_RIGHT_THUMBSTICK_DOWN;
		case VK_PAD_RTHUMB_DOWNRIGHT: return Key::GAMEPAD_RIGHT_THUMBSTICK_DOWNRIGHT;
		case VK_PAD_RTHUMB_DOWNLEFT: return Key::GAMEPAD_RIGHT_THUMBSTICK_DOWNLEFT;
		case VK_PAD_RTHUMB_RIGHT: return Key::GAMEPAD_RIGHT_THUMBSTICK_RIGHT;
		case VK_PAD_RTHUMB_LEFT: return Key::GAMEPAD_RIGHT_THUMBSTICK_LEFT;
		default: return Key::UNASSIGNED;
	}
}

inline Key TranslateWin32VirtualKeys(WPARAM vkey)
{
	switch (vkey) {
		case 0x00: return Key::UNASSIGNED;
		case VK_LBUTTON: return Key::LBUTTON;
		case VK_RBUTTON: return Key::RBUTTON;
		case VK_CANCEL: return Key::CANCEL;
		case VK_MBUTTON: return Key::MBUTTON;
		case VK_XBUTTON1: return Key::XBUTTON01;
		case VK_XBUTTON2: return Key::XBUTTON02;
		case 0x07: return Key::RESERVED;
		case VK_BACK: return Key::BACKSPACE;
		case VK_TAB: return Key::TAB;
		case 0x0A:
		case 0x0B: return Key::RESERVED;
		case VK_CLEAR: return Key::CLEAR;
		case VK_RETURN: return Key::ENTER;
		case 0x0E:
		case 0x0F: return Key::UNASSIGNED;
		case VK_SHIFT: return Key::SHIFT;
		case VK_CONTROL: return Key::CTRL;
		case VK_MENU: return Key::ALT;
		case VK_PAUSE: return Key::PAUSE;
		case VK_CAPITAL: return Key::CAPSLOCK;
		case 0x15: return Key::HANGUL;
		case 0x16: return Key::UNASSIGNED;
		case VK_JUNJA: return Key::JUNJA;
		case VK_FINAL: return Key::FINAL;
		case 0x19: return Key::KANJI;
		case 0x1A: return Key::UNASSIGNED;
		case VK_ESCAPE: return Key::ESC;
		case VK_CONVERT: return Key::CONVERT;
		case VK_NONCONVERT: return Key::NONCONVERT;
		case VK_ACCEPT: return Key::ACCEPT;
		case VK_MODECHANGE: return Key::MODECHANGE;
		case VK_SPACE: return Key::SPACEBAR;
		case VK_PRIOR: return Key::PAGEUP;
		case VK_NEXT: return Key::PAGEDOWN;
		case VK_END: return Key::END;
		case VK_HOME: return Key::HOME;
		case VK_LEFT: return Key::LEFT;
		case VK_UP: return Key::UP;
		case VK_RIGHT: return Key::RIGHT;
		case VK_DOWN: return Key::DOWN;
		case VK_SELECT: return Key::SELECT;
		case VK_PRINT: return Key::PRINT;
		case VK_EXECUTE: return Key::EXECUTE;
		case VK_SNAPSHOT: return Key::PRINTSCREEN;
		case VK_INSERT: return Key::INSERT;
		case VK_DELETE: return Key::DEL;
		case VK_HELP: return Key::HELP;
		case 0x30: return Key::K0;
		case 0x31: return Key::K1;
		case 0x32: return Key::K2;
		case 0x33: return Key::K3;
		case 0x34: return Key::K4;
		case 0x35: return Key::K5;
		case 0x36: return Key::K6;
		case 0x37: return Key::K7;
		case 0x38: return Key::K8;
		case 0x39: return Key::K9;
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3E:
		case 0x3F:
		case 0x40: return Key::UNASSIGNED;
		case 0x41: return Key::A;
		case 0x42: return Key::B;
		case 0x43: return Key::C;
		case 0x44: return Key::D;
		case 0x45: return Key::E;
		case 0x46: return Key::F;
		case 0x47: return Key::G;
		case 0x48: return Key::H;
		case 0x49: return Key::I;
		case 0x4A: return Key::J;
		case 0x4B: return Key::K;
		case 0x4C: return Key::L;
		case 0x4D: return Key::M;
		case 0x4E: return Key::N;
		case 0x4F: return Key::O;
		case 0x50: return Key::P;
		case 0x51: return Key::Q;
		case 0x52: return Key::R;
		case 0x53: return Key::S;
		case 0x54: return Key::T;
		case 0x55: return Key::U;
		case 0x56: return Key::V;
		case 0x57: return Key::W;
		case 0x58: return Key::X;
		case 0x59: return Key::Y;
		case 0x5A: return Key::Z;
		case VK_LWIN: return Key::LWINDOWS;
		case VK_RWIN: return Key::RWINDOWS;
		case VK_APPS: return Key::APPS;
		case 0x5E: return Key::RESERVED;
		case VK_SLEEP: return Key::SLEEP;
		case VK_NUMPAD0: return Key::NUM0;
		case VK_NUMPAD1: return Key::NUM1;
		case VK_NUMPAD2: return Key::NUM2;
		case VK_NUMPAD3: return Key::NUM3;
		case VK_NUMPAD4: return Key::NUM4;
		case VK_NUMPAD5: return Key::NUM5;
		case VK_NUMPAD6: return Key::NUM6;
		case VK_NUMPAD7: return Key::NUM7;
		case VK_NUMPAD8: return Key::NUM8;
		case VK_NUMPAD9: return Key::NUM9;
		case VK_MULTIPLY: return Key::MULTIPLY;
		case VK_ADD: return Key::ADD;
		case VK_SEPARATOR: return Key::SEPARATOR;
		case VK_SUBTRACT: return Key::SUBTRACT;
		case VK_DECIMAL: return Key::DECIMAL;
		case VK_DIVIDE: return Key::DIVIDE;
		case VK_F1: return Key::F1;
		case VK_F2: return Key::F2;
		case VK_F3: return Key::F3;
		case VK_F4: return Key::F4;
		case VK_F5: return Key::F5;
		case VK_F6: return Key::F6;
		case VK_F7: return Key::F7;
		case VK_F8: return Key::F8;
		case VK_F9: return Key::F9;
		case VK_F10: return Key::F10;
		case VK_F11: return Key::F11;
		case VK_F12: return Key::F12;
		case VK_F13: return Key::F13;
		case VK_F14: return Key::F14;
		case VK_F15: return Key::F15;
		case VK_F16: return Key::F16;
		case VK_F17: return Key::F17;
		case VK_F18: return Key::F18;
		case VK_F19: return Key::F19;
		case VK_F20: return Key::F20;
		case VK_F21: return Key::F21;
		case VK_F22: return Key::F22;
		case VK_F23: return Key::F23;
		case VK_F24: return Key::F24;
		case VK_NAVIGATION_VIEW: return Key::NAVIGATION_VIEW;
		case VK_NAVIGATION_MENU: return Key::NAVIGATION_MENU;
		case VK_NAVIGATION_UP: return Key::NAVIGATION_UP;
		case VK_NAVIGATION_DOWN: return Key::NAVIGATION_DOWN;
		case VK_NAVIGATION_LEFT: return Key::NAVIGATION_LEFT;
		case VK_NAVIGATION_RIGHT: return Key::NAVIGATION_RIGHT;
		case VK_NAVIGATION_ACCEPT: return Key::NAVIGATION_ACCEPT;
		case VK_NAVIGATION_CANCEL: return Key::NAVIGATION_CANCEL;
		case VK_NUMLOCK: return Key::NUMLOCK;
		case VK_SCROLL: return Key::SCROLL;
		case 0x92: return Key::OEM_FJ_JISHO;
		case VK_OEM_FJ_MASSHOU: return Key::OEM_FJ_MASSHOU;
		case VK_OEM_FJ_TOUROKU: return Key::OEM_FJ_TOUROKU;
		case VK_OEM_FJ_LOYA: return Key::OEM_FJ_LOYA;
		case VK_OEM_FJ_ROYA: return Key::OEM_FJ_ROYA;
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F: return Key::UNASSIGNED;
		case VK_LSHIFT: return Key::LSHIFT;
		case VK_RSHIFT: return Key::RSHIFT;
		case VK_LCONTROL: return Key::LCONTROL;
		case VK_RCONTROL: return Key::RCONTROL;
		case VK_LMENU: return Key::LALT;
		case VK_RMENU: return Key::RALT;
		case VK_BROWSER_BACK: return Key::BROWSER_BACK;
		case VK_BROWSER_FORWARD: return Key::BROWSER_FORWARD;
		case VK_BROWSER_REFRESH: return Key::BROWSER_REFRESH;
		case VK_BROWSER_STOP: return Key::BROWSER_STOP;
		case VK_BROWSER_SEARCH: return Key::BROWSER_SEARCH;
		case VK_BROWSER_FAVORITES: return Key::BROWSER_FAVORITES;
		case VK_BROWSER_HOME: return Key::BROWSER_HOME;
		case VK_VOLUME_MUTE: return Key::VOLUME_MUTE;
		case VK_VOLUME_DOWN: return Key::VOLUME_DOWN;
		case VK_VOLUME_UP: return Key::VOLUME_UP;
		case VK_MEDIA_NEXT_TRACK: return Key::MEDIA_NEXT_TRACK;
		case VK_MEDIA_PREV_TRACK: return Key::MEDIA_PREV_TRACK;
		case VK_MEDIA_STOP: return Key::MEDIA_STOP;
		case VK_MEDIA_PLAY_PAUSE: return Key::MEDIA_PLAY_PAUSE;
		case VK_LAUNCH_MAIL: return Key::LAUNCH_MAIL;
		case VK_LAUNCH_MEDIA_SELECT: return Key::LAUNCH_MEDIA_SELECT;
		case VK_LAUNCH_APP1: return Key::LAUNCH_APP1;
		case VK_LAUNCH_APP2: return Key::LAUNCH_APP2;
		case 0xB8:
		case 0xB9: return Key::RESERVED;
		case VK_OEM_1: return Key::SEMICOL;
		case VK_OEM_PLUS: return Key::OEM_PLUS;
		case VK_OEM_COMMA: return Key::OEM_COMMA;
		case VK_OEM_MINUS: return Key::OEM_MINUS;
		case VK_OEM_PERIOD: return Key::OEM_PERIOD;
		case VK_OEM_2: return Key::OEM_2;
		case VK_OEM_3: return Key::TILDE;
		case 0xC1:
		case 0xC2: return Key::RESERVED;
		case VK_GAMEPAD_A: return Key::GAMEPAD_A;
		case VK_GAMEPAD_B: return Key::GAMEPAD_B;
		case VK_GAMEPAD_X: return Key::GAMEPAD_X;
		case VK_GAMEPAD_Y: return Key::GAMEPAD_Y;
		case VK_GAMEPAD_RIGHT_SHOULDER: return Key::GAMEPAD_RIGHT_SHOULDER;
		case VK_GAMEPAD_LEFT_SHOULDER: return Key::GAMEPAD_LEFT_SHOULDER;
		case VK_GAMEPAD_LEFT_TRIGGER: return Key::GAMEPAD_LEFT_TRIGGER;
		case VK_GAMEPAD_RIGHT_TRIGGER: return Key::GAMEPAD_RIGHT_TRIGGER;
		case VK_GAMEPAD_DPAD_UP: return Key::GAMEPAD_DPAD_UP;
		case VK_GAMEPAD_DPAD_DOWN: return Key::GAMEPAD_DPAD_DOWN;
		case VK_GAMEPAD_DPAD_LEFT: return Key::GAMEPAD_DPAD_LEFT;
		case VK_GAMEPAD_DPAD_RIGHT: return Key::GAMEPAD_DPAD_RIGHT;
		case VK_GAMEPAD_MENU: return Key::GAMEPAD_MENU;
		case VK_GAMEPAD_VIEW: return Key::GAMEPAD_VIEW;
		case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON: return Key::GAMEPAD_LEFT_THUMBSTICK_BUTTON;
		case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON: return Key::GAMEPAD_RIGHT_THUMBSTICK_BUTTON;
		case VK_GAMEPAD_LEFT_THUMBSTICK_UP: return Key::GAMEPAD_LEFT_THUMBSTICK_UP;
		case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN: return Key::GAMEPAD_LEFT_THUMBSTICK_DOWN;
		case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT: return Key::GAMEPAD_LEFT_THUMBSTICK_RIGHT;
		case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT: return Key::GAMEPAD_LEFT_THUMBSTICK_LEFT;
		case VK_GAMEPAD_RIGHT_THUMBSTICK_UP: return Key::GAMEPAD_RIGHT_THUMBSTICK_UP;
		case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN: return Key::GAMEPAD_RIGHT_THUMBSTICK_DOWN;
		case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT: return Key::GAMEPAD_RIGHT_THUMBSTICK_RIGHT;
		case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT: return Key::GAMEPAD_RIGHT_THUMBSTICK_LEFT;
		case VK_OEM_4: return Key::OEM_4;
		case VK_OEM_5: return Key::OEM_5;
		case VK_OEM_6: return Key::OEM_6;
		case VK_OEM_7: return Key::OEM_7;
		case VK_OEM_8: return Key::OEM_8;
		case 0xE0: return Key::RESERVED;
		case VK_OEM_AX: return Key::OEM_AX;
		case VK_OEM_102: return Key::OEM_102;
		case VK_ICO_HELP: return Key::ICO_HELP;
		case VK_ICO_00: return Key::ICO_00;
		case VK_PROCESSKEY: return Key::PROCESSKEY;
		case VK_ICO_CLEAR: return Key::ICO_CLEAR;
		case VK_PACKET: return Key::PACKET;
		case 0xE8: return Key::UNASSIGNED;
		case VK_OEM_RESET: return Key::OEM_RESET;
		case VK_OEM_JUMP: return Key::OEM_JUMP;
		case VK_OEM_PA1: return Key::OEM_PA1;
		case VK_OEM_PA2: return Key::OEM_PA2;
		case VK_OEM_PA3: return Key::OEM_PA3;
		case VK_OEM_WSCTRL: return Key::OEM_WSCTRL;
		case VK_OEM_CUSEL: return Key::OEM_CUSEL;
		case VK_OEM_ATTN: return Key::OEM_ATTN;
		case VK_OEM_FINISH: return Key::OEM_FINISH;
		case VK_OEM_COPY: return Key::OEM_COPY;
		case VK_OEM_AUTO: return Key::OEM_AUTO;
		case VK_OEM_ENLW: return Key::OEM_ENLW;
		case VK_OEM_BACKTAB: return Key::OEM_BACKTAB;
		case VK_ATTN: return Key::ATTN;
		case VK_CRSEL: return Key::CRSEL;
		case VK_EXSEL: return Key::EXSEL;
		case VK_EREOF: return Key::EREOF;
		case VK_PLAY: return Key::PLAY;
		case VK_ZOOM: return Key::ZOOM;
		case VK_NONAME: return Key::NONAME;
		case VK_PA1: return Key::PA1;
		case VK_OEM_CLEAR: return Key::OEM_CLEAR;
		case 0xFF: return Key::RESERVED;
		default: return Key::UNASSIGNED;
	}
}
