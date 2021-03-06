
// ControllerStrings.cpp
//
// Copyright (c) 2010 Ryan Curtin.
// Copyright (c) 2013, 2014 Michael Imamura.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which
// you have taken this file. If you can not find the license you can not use
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions
// and limitations under the License.

#include <unordered_map>

#include "Controller.h"

namespace HoverRace {
namespace Control {

namespace {

class KeyToString
{
	/// Keycode to user-visible name.
	std::unordered_map<SDL_Keycode, std::string> k2s;
	/// Canonical key name to keycode.
	std::unordered_map<std::string, SDL_Keycode> s2k;
	const std::string unknown;

public:
	KeyToString &operator=(const KeyToString&) = delete;

protected:
	template<class Name>
	void MapKey(SDL_Keycode key, Name &&name)
	{
		k2s.emplace(key, std::forward<Name>(name));
	}

	template<class Name, class Val>
	void MapKey(SDL_Keycode key, Name &&name, Val &&val)
	{
		MapKey(key, std::forward<Name>(name));
		s2k.emplace(std::forward<Val>(val), key);
	}

public:
	KeyToString() : unknown(_("Unknown"))
	{
		// Note: By convention, the user-visible key names are
		//       mixed-case, while the canonical key names are
		//       lower-case with underscores.

		MapKey(SDLK_UNKNOWN, _("Unknown"));
		MapKey(SDLK_BACKSPACE, _("Backspace"), "backspace");
		MapKey(SDLK_TAB, _("Tab"), "tab");
		MapKey(SDLK_RETURN, _("Enter"), "enter");
		MapKey(SDLK_ESCAPE, _("Esc"), "esc");
		MapKey(SDLK_SPACE, _("Space"), "space");
		for (SDL_Keycode i = SDLK_EXCLAIM; i <= SDLK_AT; i++) {
			std::string c(1, static_cast<char>(i));
			MapKey(i, c, c);
		}
		for (SDL_Keycode i = SDLK_LEFTBRACKET; i <= SDLK_BACKQUOTE; i++) {
			std::string c(1, static_cast<char>(i));
			MapKey(i, c, c);
		}
		for (SDL_Keycode i = SDLK_a; i <= SDLK_z; i++) {
			MapKey(i,
				std::string(1, static_cast<char>(i - 32)),
				std::string(1, static_cast<char>(i)));
		}
		MapKey(SDLK_DELETE, _("Del"), "del");
		MapKey(SDLK_CAPSLOCK, _("Caps Lock"), "caps_lock");
		MapKey(SDLK_F1, _("F1"), "f1");
		MapKey(SDLK_F2, _("F2"), "f2");
		MapKey(SDLK_F3, _("F3"), "f3");
		MapKey(SDLK_F4, _("F4"), "f4");
		MapKey(SDLK_F5, _("F5"), "f5");
		MapKey(SDLK_F6, _("F6"), "f6");
		MapKey(SDLK_F7, _("F7"), "f7");
		MapKey(SDLK_F8, _("F8"), "f8");
		MapKey(SDLK_F9, _("F9"), "f9");
		MapKey(SDLK_F10, _("F10"), "f10");
		MapKey(SDLK_F11, _("F11"), "f11");
		MapKey(SDLK_F12, _("F12"), "f12");
		MapKey(SDLK_PRINTSCREEN, _("Print Screen"), "prtscr");
		MapKey(SDLK_SCROLLLOCK, _("Scroll Lock"), "scroll_lock");
		MapKey(SDLK_PAUSE, _("Pause"), "pause");
		MapKey(SDLK_INSERT, _("Ins"), "ins");
		MapKey(SDLK_HOME, _("Home"), "home");
		MapKey(SDLK_PAGEUP, _("PgUp"), "pgup");
		MapKey(SDLK_END, _("End"), "end");
		MapKey(SDLK_PAGEDOWN, _("PgDn"), "pgdn");
		MapKey(SDLK_RIGHT, std::string("\xe2\x86\x92"));  // UTF-8: RIGHTWARDS ARROW
		MapKey(SDLK_LEFT, std::string("\xe2\x86\x90"));   // UTF-8: LEFTWARDS ARROW
		MapKey(SDLK_DOWN, std::string("\xe2\x86\x93"));   // UTF-8: DOWNWARDS ARROW
		MapKey(SDLK_UP, std::string("\xe2\x86\x91"));     // UTF-8: UPWARDS ARROW
		MapKey(SDLK_NUMLOCKCLEAR, _("Num Lock"), "num_lock");
		MapKey(SDLK_KP_DIVIDE, _("Numpad /"));
		MapKey(SDLK_KP_MULTIPLY, _("Numpad *"));
		MapKey(SDLK_KP_MINUS, _("Numpad -"));
		MapKey(SDLK_KP_PLUS, _("Numpad +"));
		MapKey(SDLK_KP_ENTER, _("Numpad Enter"));
		MapKey(SDLK_KP_1, _("Numpad 1"));
		MapKey(SDLK_KP_2, _("Numpad 2"));
		MapKey(SDLK_KP_3, _("Numpad 3"));
		MapKey(SDLK_KP_4, _("Numpad 4"));
		MapKey(SDLK_KP_5, _("Numpad 5"));
		MapKey(SDLK_KP_6, _("Numpad 6"));
		MapKey(SDLK_KP_7, _("Numpad 7"));
		MapKey(SDLK_KP_8, _("Numpad 8"));
		MapKey(SDLK_KP_9, _("Numpad 9"));
		MapKey(SDLK_KP_0, _("Numpad 0"));
		MapKey(SDLK_KP_PERIOD, _("Numpad ."));
		MapKey(SDLK_APPLICATION, _("App"));
		MapKey(SDLK_POWER, _("Power"));
		MapKey(SDLK_KP_EQUALS, _("Numpad ="));
		MapKey(SDLK_F13, _("F13"), "f13");
		MapKey(SDLK_F14, _("F14"), "f14");
		MapKey(SDLK_F15, _("F15"), "f15");
		MapKey(SDLK_F16, _("F16"), "f16");
		MapKey(SDLK_F17, _("F17"), "f17");
		MapKey(SDLK_F18, _("F18"), "f18");
		MapKey(SDLK_F19, _("F19"), "f19");
		MapKey(SDLK_F20, _("F20"), "f20");
		MapKey(SDLK_F21, _("F21"), "f21");
		MapKey(SDLK_F22, _("F22"), "f22");
		MapKey(SDLK_F23, _("F23"), "f23");
		MapKey(SDLK_F24, _("F24"), "f24");
		MapKey(SDLK_EXECUTE, _("Execute"));
		MapKey(SDLK_HELP, _("Help"));
		MapKey(SDLK_MENU, _("Menu"));
		MapKey(SDLK_SELECT, _("Select"));
		MapKey(SDLK_STOP, _("Stop"));
		MapKey(SDLK_AGAIN, _("Again"));
		MapKey(SDLK_UNDO, _("Undo"));
		MapKey(SDLK_CUT, _("Cut"));
		MapKey(SDLK_COPY, _("Copy"));
		MapKey(SDLK_PASTE, _("Paste"));
		MapKey(SDLK_FIND, _("Find"));
		MapKey(SDLK_MUTE, _("Mute"));
		MapKey(SDLK_VOLUMEUP, _("Volume Up"));
		MapKey(SDLK_VOLUMEDOWN, _("Volume Down"));
		MapKey(SDLK_KP_COMMA, _("Numpad ,"));
		//MapKey(SDLK_KP_EQUALSAS400, _(""));
		//MapKey(SDLK_ALTERASE, _("Alt Erase"));
		MapKey(SDLK_SYSREQ, _("SysReq"));
		MapKey(SDLK_CANCEL, _("Cancel"));
		MapKey(SDLK_CLEAR, _("Clear"));
		MapKey(SDLK_PRIOR, _("Prior"));
		//MapKey(SDLK_RETURN2, _("Return 2"));
		//MapKey(SDLK_SEPARATOR, _("Separator"));
		//MapKey(SDLK_OUT, _("Out"));
		//MapKey(SDLK_OPER, _("Oper"));
		//MapKey(SDLK_CLEARAGAIN, _("Clear Again"));
		//MapKey(SDLK_CRSEL, _("CrSel"));
		//MapKey(SDLK_EXSEL, _("ExSel"));
		MapKey(SDLK_KP_00, _("Numpad 00"));
		MapKey(SDLK_KP_000, _("Numpad 000"));
		MapKey(SDLK_LCTRL, _("Left Ctrl"));
		MapKey(SDLK_LSHIFT, _("Left Shift"));
		MapKey(SDLK_LALT, _("Left Alt"));
		MapKey(SDLK_LGUI, _("Left GUI"));
		MapKey(SDLK_RCTRL, _("Right Ctrl"));
		MapKey(SDLK_RSHIFT, _("Right Shift"));
		MapKey(SDLK_RALT, _("Right Alt"));
		MapKey(SDLK_RGUI, _("Right GUI"));
		MapKey(SDLK_MODE, _("Mode"));
		MapKey(SDLK_AUDIONEXT, _("Next Track"));
		MapKey(SDLK_AUDIOPREV, _("Prev Track"));
		MapKey(SDLK_AUDIOSTOP, _("Stop"));
		MapKey(SDLK_AUDIOPLAY, _("Play"));
		MapKey(SDLK_AUDIOMUTE, _("Mute"));
		MapKey(SDLK_MEDIASELECT, _("Media Select"));
	}

	const std::string &Lookup(SDL_Keycode key) const
	{
		auto iter = k2s.find(key);
		return (iter == k2s.end()) ? unknown : iter->second;
	}

	SDL_Keycode Lookup(const std::string &s) const
	{
		auto iter = s2k.find(s);
		return (iter == s2k.end()) ? SDLK_UNKNOWN : iter->second;
	}
};

const KeyToString &GetKeyToString()
{
	static const KeyToString keyToString;
	return keyToString;
}

}  // namespace

/**
 * Convert a hash into an internationalized string.
 * @param hash The control hash, from one of the hash functions,
 *             e.g. HashKeyboardEvent.
 * @return The translated string.
 */
std::string InputEventController::HashToString(int hash)
{
	const KeyToString &keyToString = GetKeyToString();

	switch((hash & 0x00C00000) >> 22) {
		case 0: // keyboard keycode event
			if((hash & 0x0000FF00) == 0)
				return _("Disabled");
			else {
				if (hash & 0x100000) {
					// Scancode as keycode.
					return keyToString.Lookup(
						static_cast<SDL_Keycode>(((hash & 0x000FFF00) >> 8) | SDLK_SCANCODE_MASK));
				}
				else {
					// Simple keycode.
					return keyToString.Lookup(static_cast<SDL_Keycode>((hash & 0x0000FF00) >> 8));
				}
			}

		case 1: // mouse event
			if((hash & 0x00300000) == 0) {
				switch((hash & 0x000FF000) >> 12) {
					case SDL_BUTTON_LEFT:
						return _("Left Mouse Btn");
					case SDL_BUTTON_RIGHT:
						return _("Right Mouse Btn");
					case SDL_BUTTON_MIDDLE:
						return _("Middle Mouse Btn");
					case SDL_BUTTON_X1:
						return _("Mouse Button 4");
					case SDL_BUTTON_X2:
						return _("Mouse Button 5");
					default:
						return _("Unknown Mouse Button");
				}
			} else {
				switch((hash & 0x000F0000) >> 16) {
					case AXIS_X:
						if(((hash & 0x0000F000) >> 12) == 1)
							return _("Mouse X+ Axis");
						else
							return _("Mouse X- Axis");
					case AXIS_Y:
						if(((hash & 0x0000F000) >> 12) == 1)
							return _("Mouse Y+ Axis");
						else
							return _("Mouse Y- Axis");
					case AXIS_Z:
						if(((hash & 0x0000F000) >> 12) == 1)
							return _("Mouse Z+ Axis");
						else
							return _("Mouse Z- Axis");
					case AXIS_WHEEL_X:
						if(((hash & 0x0000F000) >> 12) == 1)
							return _("Mouse Wheel Right");
						else
							return _("Mouse Wheel Left");
					case AXIS_WHEEL_Y:
						if(((hash & 0x0000F000) >> 12) == 1)
							return _("Mouse Wheel Down");
						else
							return _("Mouse Wheen Up");
					default:
						if(((hash & 0x0000F000) >> 12) == 1)
							return _("Unknown Mouse Axis +");
						else
							return _("Unknown Mouse Axis -");
				}
			}
		/*
		case 2: // joystick event
			char joynum[100];
			sprintf(joynum, "%s %d", _("Joystick"), (hash & 0x000FF000) >> 12);

			if((hash & 0x00300000) == 0) {
				// joystick button
				char ret[100];
				sprintf(ret, "%s %s %d\0", joynum, _("Button"), (hash & 0x00000FF0) >> 4);
				return ret;
			} else if(((hash & 0x00300000) >> 20) == 1) {
				// joystick slider
				char slider[100];
				sprintf(slider, "%s %s %d", joynum, _("Slider"), (hash & 0x00000FF0) >> 4);
				return slider;
			} else if(((hash & 0x00300000) >> 20) == 2) {
				// joystick pov
				char *povnum;
				switch((hash & 0x000000F0) >> 4) { // try to use longer gettext phrases
					case 1:
						povnum = _("POV 1");
						break;
					case 2:
						povnum = _("POV 2");
						break;
					case 3:
						povnum = _("POV 3");
						break;
					case 4:
						povnum = _("POV 4");
						break;
				}
				// now the direction
				char *dir;
				switch((hash & 0x00000F00) >> 8) {
					case Pov::Centered:
						dir = _("Center");
						break;
					case Pov::North:
						dir = _("North");
						break;
					case Pov::East:
						dir = _("East");
						break;
					case Pov::West:
						dir = _("West");
						break;
					case Pov::South:
						dir = _("South");
						break;
					case Pov::NorthEast:
						dir = _("Northeast");
						break;
					case Pov::NorthWest:
						dir = _("Northwest");
						break;
					case Pov::SouthEast:
						dir = _("Southeast");
						break;
					case Pov::SouthWest:
						dir = _("Southwest");
						break;
					default:
						dir = _("Unknown direction");
						break;
				}
				char ret[150];
				sprintf(ret, "%s %s %s\0", joynum, povnum, dir);
				return ret;
			} else {
				// joystick axis
				char joyax[150];
				sprintf(joyax, "%s %s %d%c",
					joynum,
					_("Axis"),
					(hash & 0x00000F00) >> 8,
					(((hash & 0x000000F0) >> 4) == 1) ? '+' : '-');
				return joyax;
			}
		*/
	}

	return _("Unknown");
}

/**
 * Convert a canonical key name into a keycode.
 * The canonical key name is used in scripts and isn't translated.
 * @param s The key name.
 * @return The keycode, or @c SDLK_UNKNOWN if the key name is unknown.
 */
SDL_Keycode InputEventController::StringToKey(const std::string &s)
{
	return GetKeyToString().Lookup(s);
}

}  // namespace Control
}  // namespace HoverRace
