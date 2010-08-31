// ControlAction.h
// Define an action to be performed by a control.  This is essentially a functor,
// and should be bound to a control.
//
// Copyright (c) 2010 Ryan Curtin
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

#ifndef __CONTROL_ACTION_H
#define __CONTROL_ACTION_H

#include <string>

namespace HoverRace {
namespace Client {
namespace Control {

/***
 * \class ControlAction
 *
 * This is an abstract base class which allows us a simple reference to arbitrary
 * functors of type ControlActionImpl (which bind a class via a template
 * parameter).  Unfortunately, this must use virtual functions to work correctly,
 * which is not optimal, but is the best the language can do.
 *
 * To create a functor, derive the ControlAction class.
 */
class ControlAction {
	public:
		ControlAction(std::string name) : name(name) { }
		virtual ~ControlAction() { }

		virtual void operator()(int eventValue) = 0;

		std::string getName() { return name; }
		
	protected:
		std::string name;
};

/***
 * \class BlankAction
 * An action performer that does absolutely nothing.
 * Provides a template for other classes that will be used as an
 * ActionPerformer.
 */
class BlankAction : public ControlAction {
	public:
		BlankAction(std::string name) : ControlAction(name) { }
		virtual void operator()(int value) { }
};

} // namespace Control
} // namespace Client
} // namespace HoverRace

#endif