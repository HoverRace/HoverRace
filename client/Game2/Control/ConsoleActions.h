// ConsoleActions.h
// A collection of classes implementing the ConsoleAction template class
// (just has to implement operator() and a constructor).  These are all
// actions to be performed when the user presses a control key.
//
// Copyright (c) 2010, Ryan Curtin
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
//

#ifndef __CONTROL_ACTIONS_H
#define __CONTROL_ACTIONS_H

#include "Controller.h"
#include "ControlAction.h"
#include "../HoverScript/HighConsole.h"

namespace HoverRace {
namespace Client {
namespace Control {

/***
 * \class ConsoleAction
 *
 * Base class for events pertaining to the console.
 */
class ConsoleAction : public ControlAction {
	public:
		ConsoleAction(std::string name, int listOrder, HoverScript::HighConsole* hc);
		virtual ~ConsoleAction() { }

		void SetHighConsole(HoverScript::HighConsole* hc);
		virtual void operator()(int value) = 0;

	protected:
		// TODO: shared_ptr
		HoverScript::HighConsole* hc;
};

/***
 * \class ConsoleKeyAction
 *
 * Types a key into the console.
 */
class ConsoleKeyAction : public ConsoleAction {
	public:
		ConsoleKeyAction(std::string name, int listOrder, HoverScript::HighConsole* hc, OIS::KeyCode kc);
		virtual ~ConsoleKeyAction() { }

		void SetHighConsole(HoverScript::HighConsole* hc);

		/***
		 * eventValue signifies if the key was pressed or released.
		 * This will only fire when eventValue > 0.
		 */
		virtual void operator()(int eventValue);

	protected:
		OIS::KeyCode kc;
};

/***
 * \class ConsoleToggleAction
 *
 * Toggles the console on and off.
 */
class ConsoleToggleAction : public ConsoleAction {
	public:
		ConsoleToggleAction(std::string name, int listOrder, HoverScript::HighConsole* hc, InputEventController* controller) : 
			ConsoleAction(name, listOrder, hc), controller(controller) { }
		virtual ~ConsoleToggleAction() { }

		/***
		 * eventValue signifies if the key was pressed or released.
		 * This will only toggle the console when eventValue > 0.
		 */
		virtual void operator()(int eventValue);

	protected:
		std::vector<std::string> oldMaps; /// To keep track of the maps we were using
		InputEventController* controller;
};

/***
 * \class ConsoleScrollAction
 *
 * Scrolls up or down in the console.
 */
class ConsoleScrollAction : public ConsoleAction {
	public:
		ConsoleScrollAction(std::string name, int listOrder, HoverScript::HighConsole* hc, int scrollStep) : 
			ConsoleAction(name, listOrder, hc), scrollStep(scrollStep) { }
		virtual ~ConsoleScrollAction() { }

		/***
		 * eventValue signifies if the key was pressed or released.
		 * This will only scroll when eventValue > 0.
		 */
		virtual void operator()(int eventValue);

	private:
		int scrollStep;
};

/***
 * \class ConsoleScrollTopAction
 *
 * Scrolls to the top of the console.
 */
class ConsoleScrollTopAction : public ConsoleAction {
	public:
		ConsoleScrollTopAction(std::string name, int listOrder, HoverScript::HighConsole* hc) :
			ConsoleAction(name, listOrder, hc) { }
		virtual ~ConsoleScrollTopAction() { }

		/***
		 * eventValue signifies if the key was pressed or released.
		 * This will only scroll when eventValue > 0.
		 */
		virtual void operator()(int eventValue);
};

/***
 * \class ConsoleScrollBottomAction
 *
 * Scrolls to the bottom of the console.
 */
class ConsoleScrollBottomAction : public ConsoleAction {
	public:
		ConsoleScrollBottomAction(std::string name, int listOrder, HoverScript::HighConsole* hc) :
			ConsoleAction(name, listOrder, hc) { }
		virtual ~ConsoleScrollBottomAction() { }

		/***
		 * eventValue signifies if the key was pressed or released.
		 * This will only scroll when eventValue > 0.
		 */
		virtual void operator()(int eventValue);
};

/***
 * \class ConsoleHelpAction
 *
 * Toggles console help.
 */
class ConsoleHelpAction : public ConsoleAction {
	public:
		ConsoleHelpAction(std::string name, int listOrder, HoverScript::HighConsole* hc) :
			ConsoleAction(name, listOrder, hc) { }
		virtual ~ConsoleHelpAction() { }

		/***
		 * eventValue signifies if the key was pressed or released.
		 * This will only scroll when eventValue > 0.
		 */
		virtual void operator()(int eventValue);
};

} // namespace Control
} // namespace Client
} // namespace HoverRace

#endif