// ActionPerformers.h
// A collection of classes implementing the ActionPerformer template class
// (just has to implement Perform() and a constructor).  These are all
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

#ifndef __ACTION_PERFORMERS_H
#define __ACTION_PERFORMERS_H

#include "Controller.h"
#include "ControlAction.h"
#include "../../../engine/MainCharacter/MainCharacter.h"
#include "../HoverScript/HighConsole.h"
#include "../Observer.h"

namespace HoverRace {
namespace Client {
namespace Control {

/***
 * \class PlayerEffectAction
 *
 * Base class for actions that affect a player.  This gives us a unified
 * constructor so we can hang on to a shared pointer to the player.  For speed
 * concerns, most of these do not check if the pointer is NULL.  Given that that
 * condition should never happen, this should not be a problem.
 */
class PlayerEffectAction : public ControlAction {
	public:
		// C++0x should give us constructor inheritance... whenever that happens
		PlayerEffectAction(std::string name, int listOrder, MainCharacter::MainCharacter* mc);

		void SetMainCharacter(MainCharacter::MainCharacter* mc);
		virtual void operator()(int value) = 0;

	protected:
		// TODO: shared_ptr
		MainCharacter::MainCharacter* mc;
};

/***
 * \class EngineAction
 * 
 * Turns the motor on or off.
 */
class EngineAction : public PlayerEffectAction {
	public:
		EngineAction(std::string name, int listOrder, MainCharacter::MainCharacter* mc) : PlayerEffectAction(name, listOrder, mc) { }

		/***
		 * eventValue > 0: turn engine on.
		 * eventValue == 0: turn engine off.
		 */
		virtual void operator()(int eventValue);
};

/***
 * \class TurnLeftAction
 *
 * Turns the craft left.
 */
class TurnLeftAction : public PlayerEffectAction {
	public:
		TurnLeftAction(std::string name, int listOrder, MainCharacter::MainCharacter* mc) : PlayerEffectAction(name, listOrder, mc) { }

		/***
		 * eventValue > 0: set craft to turn left
		 * eventValue == 0: stop left turn
		 */
		virtual void operator()(int eventValue);
};

/***
 * \class TurnRightAction
 *
 * Turns the craft right.
 */
class TurnRightAction : public PlayerEffectAction {
	public:
		TurnRightAction(std::string name, int listOrder, MainCharacter::MainCharacter* mc) : PlayerEffectAction(name, listOrder, mc) { }
		
		/***
		 * eventValue > 0: set craft to turn right
		 * eventValue == 0: stop right turn
		 */
		virtual void operator()(int eventValue);
};

/***
 * \class JumpAction
 *
 * Makes the craft jump.
 */
class JumpAction : public PlayerEffectAction {
	public:
		JumpAction(std::string name, int listOrder, MainCharacter::MainCharacter* mc) : PlayerEffectAction(name, listOrder, mc) { }
		
		/***
		 * The craft will be told to jump, regardless of eventValue.
		 */
		virtual void operator()(int eventValue);
};

/***
 * \class PowerupAction
 *
 * Fires a missile, lays a mine, or uses a speed can, depending on the user's
 * selected item.
 */
class PowerupAction : public PlayerEffectAction {
	public:
		PowerupAction(std::string name, int listOrder, MainCharacter::MainCharacter* mc) : PlayerEffectAction(name, listOrder, mc) { }
		
		/***
		 * Uses the item, regardless of eventValue.
		 */
		virtual void operator()(int eventValue);
};

/***
 * \class ChangeItemAction
 *
 * Changes the current item.
 */
class ChangeItemAction : public PlayerEffectAction {
	public:	
		ChangeItemAction(std::string name, int listOrder, MainCharacter::MainCharacter* mc) : PlayerEffectAction(name, listOrder, mc) { }
		
		/***
		 * Changes the current item, regardless of eventValue.
		 */
		virtual void operator()(int eventValue);
};

/***
 * \class BrakeAction
 *
 * Brakes the craft (or turns it around so that if used in conjunction with the engine,
 * the craft slows down).
 */
class BrakeAction : public PlayerEffectAction {
	public:
		BrakeAction(std::string name, int listOrder, MainCharacter::MainCharacter* mc) : PlayerEffectAction(name, listOrder, mc) { }
		
		/***
		 * eventValue > 0: set brake state on
		 * eventValue == 0: set brake state off
		 */
		virtual void operator()(int eventValue);
};

/***
 * \class LookBackAction
 *
 * Sets whether or not the player is looking back.  This is arguably not a
 * PlayerEffectAction but we'll leave it here for now.
 */
class LookBackAction : public PlayerEffectAction {
	public:
		LookBackAction(std::string name, int listOrder, MainCharacter::MainCharacter* mc) : PlayerEffectAction(name, listOrder, mc) { }
		
		/***
		 * eventValue > 0: set lookback state on
		 * eventValue == 0: set lookback state off
		 */
		virtual void operator()(int eventValue);
};

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
 * \class ObserverAction
 *
 * Base class for all camera changing or otherwise observer-related actions.
 */
class ObserverAction : public ControlAction {
	public:
		ObserverAction(std::string name, int listOrder, Observer** observers, int nObservers) : ControlAction(name, listOrder), observers(observers), nObservers(nObservers) { }
		virtual ~ObserverAction() { }

		void SetObservers(Observer** observers, int nObservers);
		virtual void operator()(int value) = 0;

	protected:
		Observer** observers;
		int nObservers;
};

/***
 * \class ObserverTiltAction
 *
 * Tilt the camera up or down (using Observer::Scroll()).
 */
class ObserverTiltAction : public ObserverAction {
	public:
		ObserverTiltAction(std::string name, int listOrder, Observer** observers, int nObservers, int tiltIncrement) : ObserverAction(name, listOrder, observers, nObservers), tiltIncrement(tiltIncrement) { }

		/// Fires on >0 value.
		virtual void operator()(int value);
	
	protected:
		int tiltIncrement;
};

/***
 * \class ObserverZoomAction
 *
 * Zoom the camera in or out.
 */
class ObserverZoomAction : public ObserverAction {
	public:
		ObserverZoomAction(std::string name, int listOrder, Observer** observers, int nObservers, int zoomIncrement) : ObserverAction(name, listOrder, observers, nObservers), zoomIncrement(zoomIncrement) { }

		/// Fires on >0 value.
		virtual void operator()(int value);

	protected:
		int zoomIncrement;
};

/***
 * \class ObserverZoomResetAction
 *
 * Reset camera zoom and tilt to default.
 */
class ObserverResetAction : public ObserverAction {
	public:
		ObserverResetAction(std::string name, int listOrder, Observer** observers, int nObservers) : ObserverAction(name, listOrder, observers, nObservers) { }

		/// Fires on >0 value.
		virtual void operator()(int value);
};

} // namespace Control
} // namespace Client
} // namespace HoverRace

#endif