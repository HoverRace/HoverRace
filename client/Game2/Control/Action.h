
// Action.h
//
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

#pragma once

#include "../../../engine/Exception.h"

#include "ControlAction.h"
#include "Controller.h"

namespace HoverRace {
namespace Client {
namespace Control {

/// Signals which are self-contained (no payload).
typedef boost::signals2::signal<void()> voidSignal_t;

/// Signals which have a single (action-dependent) payload.
typedef boost::signals2::signal<void(int)> valueSignal_t;

template<class T>
inline void PerformAction(const T&, int)
{
	std::ostringstream oss;
	oss << "PerformAction<" << typeid(T).name() << '>';
	throw UnimplementedExn(oss.str());
}

template<>
inline void PerformAction<voidSignal_t>(const voidSignal_t &signal, int value)
{
	// voidSignals are only triggered if the value is > 0.
	// For keys and buttons, that means key-down / button-down.
	if (value > 0) {
		signal();
	}
}

template<>
inline void PerformAction<valueSignal_t>(const valueSignal_t &signal, int value)
{
	signal(value);
}

/**
 * Actions are effectively signals that can be triggered by both the
 * InputEventController and other things such as UI buttons.
 *
 * Actions also keep track of what key/button is assigned to it for UI
 * components to display to the user.
 *
 * @tparam T The signal type.
 * @author Michael Imamura
 */
template<class T, class Val=int>
class Action : public ControlAction<Val>
{
	typedef ControlAction<Val> SUPER;
	public:
		Action(const std::string &name, int listOrder) :
			SUPER(name, listOrder), primaryTrigger(0) { }
		virtual ~Action() { }

		virtual void operator()(Val value) { PerformAction<T>(signal, value); }

		/**
		 * Connect a slot to the signal.
		 * This is a convenience function.  To access the signal directly,
		 * see GetSignal().
		 */
		template<class U>
		boost::signals2::connection Connect(U &&del) {
			return signal.connect(std::forward<U>(del));
		}

		/**
		 * Retrieve the hash of the key or button assigned to this action by
		 * the InputEventController.
		 * @return The hash (see InputEventController::HashToString) or 0 if no key
		 *         or button has been assigned.
		 */
		int GetPrimaryTrigger() const { return primaryTrigger; }

		/**
		 * Set the hash of the key or button assigned to this action.
		 * This should only be called by the InputEventController that owns this
		 * Action (if any).
		 * @param hash The input hash or 0 if none is assigned.
		 */
		void SetPrimaryTrigger(int hash) { primaryTrigger = hash; }

		T &GetSignal() { return signal; }

	private:
		int primaryTrigger;
		T signal;
};

} // namespace Control
} // namespace Client
} // namespace HoverRace
