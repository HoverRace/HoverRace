
// ControllerStrings.cpp
//
// Copyright (c) 2010 Ryan Curtin.
// Copyright (c) 2013 Michael Imamura.
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

#include "StdAfx.h"

#include <unordered_map>

#include "Controller.h"

namespace HoverRace {
namespace Client {
namespace Control {

namespace {
	class KeyToString
	{
		typedef std::unordered_map<SDL_Keycode, std::string> lookup_t;
		lookup_t lookup;
		std::string unknown;

		protected:
			void MapKey(SDL_Keycode key, std::string &&s)
			{
				lookup.emplace(lookup_t::value_type(key, std::forward<std::string>(s)));
			}
		public:
			KeyToString() : lookup(), unknown(_("Unknown"))
			{
				MapKey(SDLK_UNKNOWN, _("Unknown"));
				MapKey(SDLK_BACKSPACE, _("Backspace"));
				MapKey(SDLK_TAB, _("Tab"));
				MapKey(SDLK_RETURN, _("Enter"));
				MapKey(SDLK_ESCAPE, _("Esc"));
				MapKey(SDLK_SPACE, _("Space"));
				for (SDL_Keycode i = SDLK_EXCLAIM; i <= SDLK_AT; i++) {
					MapKey(i, std::string(1, i));
				}
				for (SDL_Keycode i = SDLK_LEFTBRACKET; i <= SDLK_BACKQUOTE; i++) {
					MapKey(i, std::string(1, i));
				}
				for (SDL_Keycode i = SDLK_a; i <= SDLK_z; i++) {
					MapKey(i, std::string(1, i - 32));
				}
				MapKey(SDLK_DELETE, _("Del"));
				MapKey(SDLK_CAPSLOCK, _("Caps Lock"));
				MapKey(SDLK_F1, _("F1"));
				MapKey(SDLK_F2, _("F2"));
				MapKey(SDLK_F3, _("F3"));
				MapKey(SDLK_F4, _("F4"));
				MapKey(SDLK_F5, _("F5"));
				MapKey(SDLK_F6, _("F6"));
				MapKey(SDLK_F7, _("F7"));
				MapKey(SDLK_F8, _("F8"));
				MapKey(SDLK_F9, _("F9"));
				MapKey(SDLK_F10, _("F10"));
				MapKey(SDLK_F11, _("F11"));
				MapKey(SDLK_F12, _("F12"));
				MapKey(SDLK_PRINTSCREEN, _("Print Screen"));
				MapKey(SDLK_SCROLLLOCK, _("Scroll Lock"));
				MapKey(SDLK_PAUSE, _("Pause"));
				MapKey(SDLK_INSERT, _("Ins"));
				MapKey(SDLK_HOME, _("Home"));
				MapKey(SDLK_PAGEUP, _("PgUp"));
				MapKey(SDLK_END, _("End"));
				MapKey(SDLK_PAGEDOWN, _("PgDn"));
				MapKey(SDLK_RIGHT, _("\xe2\x86\x92"));  // UTF-8: RIGHTWARDS ARROW
				MapKey(SDLK_LEFT, _("\xe2\x86\x90"));   // UTF-8: LEFTWARDS ARROW
				MapKey(SDLK_DOWN, _("\xe2\x86\x93"));   // UTF-8: DOWNWARDS ARROW
				MapKey(SDLK_UP, _("\xe2\x86\x91"));     // UTF-8: UPWARDS ARROW
				MapKey(SDLK_NUMLOCKCLEAR, _("Num Lock"));
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
				MapKey(SDLK_F13, _("F13"));
				MapKey(SDLK_F14, _("F14"));
				MapKey(SDLK_F15, _("F15"));
				MapKey(SDLK_F16, _("F16"));
				MapKey(SDLK_F17, _("F17"));
				MapKey(SDLK_F18, _("F18"));
				MapKey(SDLK_F19, _("F19"));
				MapKey(SDLK_F20, _("F20"));
				MapKey(SDLK_F21, _("F21"));
				MapKey(SDLK_F22, _("F22"));
				MapKey(SDLK_F23, _("F23"));
				MapKey(SDLK_F24, _("F24"));
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

			const std::string &Lookup(SDL_Keycode key)
			{
				lookup_t::iterator iter = lookup.find(key);
				return (iter == lookup.end()) ? unknown : iter->second;
			}
	};
}

std::string InputEventController::HashToString(int hash)
{
	static KeyToString keyToString;

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

		/*TODO
		case 1: // mouse event
			if((hash & 0x00300000) == 0) {
				// OIS has no nice "getAsString" for mice so we have to do it by hand
				switch((hash & 0x000FF000) >> 12) {
					case MB_Left:
						return _("Left Mouse Btn");
					case MB_Right:
						return _("Right Mouse Btn");
					case MB_Middle:
						return _("Middle Mouse Btn");
					case MB_Button3:
						return _("Mouse Button 3");
					case MB_Button4:
						return _("Mouse Button 4");
					case MB_Button5:
						return _("Mouse Button 5");
					case MB_Button6:
						return _("Mouse Button 6");
					case MB_Button7:
						return _("Mouse Button 7");
					// apparently there is no support for more than 7 mouse buttons
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
					default:
						if(((hash & 0x0000F000) >> 12) == 1)
							return _("Unknown Mouse Axis +");
						else
							return _("Unknown Mouse Axis -");
				}
			}
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

}  // namespace Control
}  // namespace HoverScript
}  // namespace Client
