
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

#include "Controller.h"

namespace HoverRace {
namespace Client {
namespace Control {

std::string InputEventController::HashToString(int hash)
{
	switch((hash & 0x00C00000) >> 22) {
		case 0: // keyboard event
			if((hash & 0x0000FF00) == 0)
				return "Disabled";
			else
				//TODO
				return "Unknown";
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
