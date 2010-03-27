
// UiHandler.h
// Base class for UI input event hookups.
//
// Copyright (c) 2010 Michael Imamura.
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

#pragma once

#include "OIS/OISKeyboard.h"
#include "OIS/OISMouse.h"
#include "OIS/OISJoyStick.h"

namespace HoverRace {
namespace Client {
namespace Control {

/**
 * Base class for UI input event hookups.
 * @author Michael Imamura
 */
class UiHandler
{
	public:
		UiHandler() { }
		virtual ~UiHandler() { }

	public:
		virtual void OnConsole() { }
};
typedef boost::shared_ptr<UiHandler> UiHandlerPtr;

}  // namespace Control
}  // namespace Client
}  // namespace HoverRace
