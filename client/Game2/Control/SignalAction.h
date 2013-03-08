
// SignalAction.h
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

#include "Controller.h"
#include "ControlAction.h"

namespace HoverRace {
namespace Client {
namespace Control {

template<class T>
void PerformSignalAction(const T &signal, int value)
{
	std::ostringstream oss;
	oss << "PerformSignalAction<" << typeid(T).name() << '>';
	throw UnimplementedExn(oss.str());
}

template<>
void PerformSignalAction<InputEventController::voidSignal_t>(const InputEventController::voidSignal_t &signal, int value)
{
	signal();
}

template<class T>
class SignalAction : public ControlAction
{
	typedef ControlAction SUPER;
	public:
		SignalAction(std::string name, int listOrder, const T &signal) :
			SUPER(name, listOrder), signal(signal) { }
		virtual ~SignalAction() { }

		virtual void operator()(int value) { PerformSignalAction<T>(signal, value); }

	protected:
		const T &signal;
};

template<class T>
SignalAction<T> *MakeSignalAction(std::string name, int listOrder, const T &signal)
{
	return new SignalAction<T>(name, listOrder, signal);
}

} // namespace Control
} // namespace Client
} // namespace HoverRace
