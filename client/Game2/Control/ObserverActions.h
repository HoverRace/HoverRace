// ObserverActions.h
// A collection of classes implementing the ObserverAction template class
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

#ifndef __OBSERVER_ACTIONS_H
#define __OBSERVER_ACTIONS_H

#include "Controller.h"
#include "ControlAction.h"
#include "../Observer.h"

namespace HoverRace {
namespace Client {
namespace Control {

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